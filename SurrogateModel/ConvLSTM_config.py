import time
import os

import numpy as np
import tensorflow as tf
from tensorflow import keras
from keras.src.utils import plot_model
from matplotlib import pyplot as plt

from ConvLSTM_model import ConvLSTM_model

##############
# DEFINITION OF BASELINE MODEL CONVLSTM
##############

class ConvLSTM_config:
    def __init__(self, dataset, dataset_train, dataset_val, dataset_test, name, batch_size, epoch, time_stride,
                 save_path, load_path, directory, initial_step_pred, pred_steps, geo_size=(64, 64),
                 learning_rate=0.003, beta_1=0.9, beta_2=0.999, batch_norm=None,
                 feats=8, padding='same',
                 loss_weights=[1, 1]):

        self.name = name
        self.loss = 'mse'
        self.loss_weights = loss_weights
        self.learning_rate = learning_rate
        self.beta_1 = beta_1
        self.beta_2 = beta_2
        self.optimizer = tf.keras.optimizers.AdamW(learning_rate=self.learning_rate, weight_decay=1e-4,
                                                   beta_1=self.beta_1, beta_2=self.beta_2, clipnorm=1.0)
        self.batch_norm = batch_norm
        self.feats = feats
        self.padding = padding

        self.metrics = ['mse', 'mse']
        self.run_eagerly = False
        self.saved_model = None
        self.save_path = save_path
        self.load_path = load_path
        self.directory = directory
        self.history = None
        self.dataset = dataset
        self.dataset_train = dataset_train
        self.dataset_val = dataset_val
        self.dataset_test = dataset_test
        self.batch_size = batch_size
        self.epoch = epoch
        self.initial_step_pred = initial_step_pred
        self.pred_steps = pred_steps  # //time_steps
        self.time_stride = time_stride
        self.train_time = 0
        self.pred_time = 0

        self.geo_size = geo_size
        self.input = input
        self.input_dict = {}
        self.model = self.model_unet()

    def model_unet(self):

        model = ConvLSTM_model(self.name, self.geo_size, self.pred_steps, self.feats,
                               self.batch_norm, self.padding)
        return model

    # --------------------------
    # CUSTOM LOSS FUNCTIONS
    # ------------------------------

    @tf.function
    def my_loss_v(self, y_true, y_pred):
        eps = 1e-6
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)

        n_true = tf.norm(y_true, axis=-1)
        n_pred = tf.norm(y_pred, axis=-1)

        # --- relative error in speed ---
        dev_veloc = tf.abs(n_pred - n_true) / (n_true + eps)

        # --- cosine similarity of velocity ---
        dot = tf.reduce_sum(y_true * y_pred, axis=-1)
        cos = dot / (n_true * n_pred + eps)
        cos = tf.clip_by_value(cos, -1.0, 1.0)
        dev_angle = tf.acos(cos) / np.pi
        # --- combination ---
        loss = 1e-5 * dev_veloc + 0.4 * dev_angle
        return tf.reduce_mean(loss)

    @tf.function  # image similarity + MAE
    def my_loss_rho(self, y_true, y_pred):
        # Mixed-precision fix
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)
        # SSIM computes image similarity
        ssim = tf.image.ssim(y_true, y_pred, max_val=1.0)
        ssim_loss = 1 - tf.reduce_mean(ssim)

        # MAE
        mae = tf.reduce_mean(tf.abs(y_true - y_pred))

        # combination
        return mae + 0.5 * ssim_loss

    @tf.function
    def mse_var_norm(self, y_true, y_pred):
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)
        mse = tf.reduce_mean(tf.square(y_true - y_pred))
        w = tf.math.log(1 + tf.math.reduce_variance(y_true))
        w = tf.maximum(w, 1e-6)
        loss = 0.3 * mse
        return loss

    @tf.function
    def h1_norm_rho(self, y_true, y_pred):
        # Mixed-precision fix
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)
        sobel_true = tf.image.sobel_edges(y_true)
        sobel_pred = tf.image.sobel_edges(y_pred)
        mse_rho = tf.reduce_mean(tf.square(y_true - y_pred))
        mse_grad_rho = tf.reduce_mean(tf.square(sobel_true - sobel_pred))
        return 1000 * mse_rho + 0.2 * mse_grad_rho

    @tf.function
    def h1_norm_v(self, y_true, y_pred):
        # Mixed-precision fix
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)

        # L2-error/MSE
        mse_v = tf.reduce_mean(tf.square(y_true - y_pred))

        # Sobel gradients
        sobel_true = tf.image.sobel_edges(y_true)  # (B,H,W,2,2,2)
        sobel_pred = tf.image.sobel_edges(y_pred)

        # L2-error gradients
        mse_grad = tf.reduce_mean(tf.square(sobel_true - sobel_pred))

        return 0.5 * mse_v + 0.1 * mse_grad

    # -----------------------------------------
    # BUILD MODEL
    # ----------------------------

    def build_model_(self, load=False):

        from ConvLSTM_model import ConvLSTM_model

        # ---------------------------------------------------------
        # 1) Load/ build model
        # ---------------------------------------------------------
        if load:
            print(f"→ Load model from {self.load_path}")

            from keras import mixed_precision

            mixed_precision.set_global_policy("float32")
            self.model = tf.keras.models.load_model(
                self.load_path,
                custom_objects={"ConvLSTM_model": ConvLSTM_model},
                compile=False
            )

            # compile
            self.model.compile(
                optimizer=self.optimizer,
                loss=[self.h1_norm_rho, self.h1_norm_v],
                loss_weights=self.loss_weights,
                metrics=[self.h1_norm_rho, self.h1_norm_v],
                jit_compile=False,
                run_eagerly=False
            )

            self.model.summary(expand_nested=True)

        else:
            # ---------------------------------------------------------
            # 2) new model
            # ---------------------------------------------------------
            print("→ Initialize new baseline ConvLSTM‑model…")

            self.model = ConvLSTM_model(
                name=self.name,
                geo_size=self.geo_size,
                pred_steps=self.pred_steps,
                feats=self.feats,
                batch_norm=self.batch_norm,
                padding=self.padding,
            )

            # ---------------------------------------------------------
            # 3) call model
            # ---------------------------------------------------------
            print("→ Build model with dummy data…")

            dummy_rho = tf.zeros((1, self.pred_steps, 64, 64, 1))
            dummy_v = tf.zeros((1, self.pred_steps, 64, 64, 2))
            dummy_geo = tf.zeros((1, self.pred_steps, 64, 64, 2))

            _ = self.model([dummy_rho, dummy_v, dummy_geo])

            # ---------------------------------------------------------
            # 4) compile model
            # ---------------------------------------------------------
            print("→ Compile model…")

            self.model.compile(
                optimizer=self.optimizer,
                loss=[self.h1_norm_rho, self.h1_norm_v],
                loss_weights=self.loss_weights,
                metrics=[self.h1_norm_rho, self.h1_norm_v],
                jit_compile=False,
                run_eagerly=False
            )

            self.model.summary(expand_nested=True)

        # ---------------------------------------------------------
        # 5) Training
        # ---------------------------------------------------------
        print("→ Start training…")

        early_stopping = tf.keras.callbacks.EarlyStopping(
            monitor='val_loss',
            patience=8,
            min_delta=1e-4,
            restore_best_weights=True
        )

        reduce_lr = tf.keras.callbacks.ReduceLROnPlateau(
            monitor="val_loss",
            factor=0.1,
            patience=2
        )

        start_timer = time.perf_counter()

        self.history = self.model.fit(
            self.dataset_train,
            epochs=self.epoch,
            batch_size=self.batch_size,
            validation_data=self.dataset_val,
            callbacks=[early_stopping, reduce_lr]
        )

        self.train_time = time.perf_counter() - start_timer
        print('→ Duration Training:', self.train_time)

        # ---------------------------------------------------------
        # 6) Save results
        # ---------------------------------------------------------
        save_dir = os.path.dirname(self.save_path)
        raw_dir = os.path.join(save_dir, "output")
        os.makedirs(raw_dir, exist_ok=True)

        raw_save_path = os.path.join(raw_dir, os.path.basename(self.save_path))

        plot_model(
            self.model,
            to_file=raw_save_path + '_model.png',
            show_shapes=True,
            expand_nested=True,
            show_layer_activations=True
        )

        plt.plot(self.history.history['loss'], label='Train')
        if 'val_loss' in self.history.history:
            plt.plot(self.history.history['val_loss'], label='Eval')
        plt.legend()
        plt.title('Loss')
        plt.savefig(raw_save_path + '_loss.png')
        plt.close()

    # ------------------------
    # PREDICT
    # ------------------------

    def predict(self):
        # falls keine test-daten
        if self.dataset_test is None:
            print("no test data")
            return

        try:
            dataset = next(iter(self.dataset_test))
        except (StopIteration, TypeError):
            print("test dataset empty")
            return

        geo = dataset[0]['geometry']
        features = dataset[0]

        rho_label = features['rho']  # (batch, 1, H, W, 1)
        v_label = features['v']  # (batch, 1, H, W, 2)

        rho_label = tf.transpose(rho_label, [0, 1, 4, 2, 3])  # (B,1,C,H,W)
        v_label = tf.transpose(v_label, [0, 1, 4, 2, 3])

        labels = (rho_label, v_label, geo)
        rho = dataset[0]['rho']
        v = dataset[0]['v']

        result_steps = self.pred_steps - self.dataset.feature_time_steps  # Schritte für Vorhersage - Inputzeitschritte

        data = {
            'rho': rho[:, :self.dataset.feature_time_steps, :, :],
            'v': v[:, :self.dataset.feature_time_steps, :, :],
            'geometry': geo[:, :self.dataset.feature_time_steps, :, :]
        }  # wird für die iterative Vorhersage genutzt, übernimmt immer nur die letzten 5 Zeitschritte
        data_temp = data  # speichert alle Zeitschritte
        start_timer = time.perf_counter()

        for step in range(result_steps - self.initial_step_pred):
            predictions = self.model.predict(data, batch_size=self.batch_size)

            new_rho = predictions[0][::, tf.newaxis, ::, ::, ::]  # zusätzliche Zeitdim. für Outputdaten erzeugen
            rho_temp = tf.concat((data_temp['rho'], new_rho), axis=1)
            new_rho = tf.concat((data['rho'], new_rho), axis=1)  # an Inputdaten anhängen

            new_v = predictions[1][::, tf.newaxis, ::, ::, ::]
            v_temp = tf.concat((data_temp['v'], new_v), axis=1)
            new_v = tf.concat((data['v'], new_v), axis=1)

            data = {
                'rho': new_rho[:, -self.dataset.feature_time_steps:, :, :],
                # letzten feature-time_steps-Zeitschritte verwenden
                'v': new_v[:, -self.dataset.feature_time_steps:, :, :],
                'geometry': geo[:, -self.dataset.feature_time_steps:, :, :]
            }

            data_temp = {'rho': rho_temp,
                         'v': v_temp,
                         'geometry': geo}

        self.pred_time = time.perf_counter() - start_timer
        print('Duration Prediction:', self.pred_time)

        predictions = (np.transpose(data_temp['rho'], [0, 1, 4, 2, 3]),
                       np.transpose(data_temp['v'], [0, 1, 4, 2, 3]))

        self.dataset.predictions = predictions
        self.dataset.labels = labels
        self.dataset.result_steps = result_steps
        # Nutze output Verzeichnis für die vielen Visualisierungen
        save_dir = os.path.dirname(self.save_path)
        raw_dir = os.path.join(save_dir, "output")
        raw_save_path = os.path.join(raw_dir, os.path.basename(self.save_path))
        self.dataset.print_label_predict_rho_v(raw_save_path)
        return self
