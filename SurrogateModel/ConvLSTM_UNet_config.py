import time
import os

import numpy as np
import tensorflow as tf

from keras.src.utils import plot_model
from matplotlib import pyplot as plt

from ConvLSTM_UNet_model import ConvLSTM_UNet_model

## Klasse legt Konfiguration des Modells fest

class ConvLSTM_UNet_config:
    def __init__(self, dataset, dataset_train, dataset_val, dataset_test, name, batch_size, epoch, time_stride,
                 save_path, load_path, directory, initial_step_pred, pred_steps, geo_size=(64, 64),
                 learning_rate=0.003, beta_1=0.9, beta_2=0.999,   batch_norm=None,
                 feats=8, kernel_size=(3, 3), pool_size=(1, 2, 2), padding='same', activation_out='tanh', return_sequence=True,
                 block_numb= 4, input=['rho','v'], output=['rho','v'],
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
        self.kernel_size = kernel_size
        self.pool_size = pool_size
        self.padding = padding
        self.activation_out = activation_out
        self.return_sequence = return_sequence

        self.metrics = ['mse' ,'mse']
        self.run_eagerly = False
        self.saved_model = None
        self.save_path = save_path
        self.load_path =load_path
        self.directory = directory
        self.history = None
        self.dataset = dataset
        self.dataset_train = dataset_train
        self.dataset_val = dataset_val
        self.dataset_test = dataset_test
        self.batch_size = batch_size
        self.epoch = epoch
        self.initial_step_pred = initial_step_pred
        self.pred_steps = pred_steps # //time_steps
        self.time_stride = time_stride
        self.train_time = 0
        self.pred_time = 0

        self.geo_size = geo_size
        self.block_numb = block_numb
        self.input = input
        self.output = output
        self.input_dict={}
        self.model = self.model_unet()

    def model_unet(self):

      model = ConvLSTM_UNet_model(self.name, self.geo_size, self.pred_steps, self.feats, self.kernel_size, self.pool_size,
                               self.batch_norm, self.padding, self.return_sequence)
      return model

    @tf.function
    def my_loss_v(self, y_true, y_pred):
        eps = 1e-6
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)

        n_true = tf.norm(y_true, axis=-1)
        n_pred = tf.norm(y_pred, axis=-1)

        # Masken
        #mask_mag = tf.cast(n_true > 0.01, tf.float32)
        #mask_ang = tf.cast(n_true > 0.05, tf.float32)

        # --- Betragsteil ---
        dev_veloc = tf.abs(n_pred - n_true) / (n_true + eps)
        #dev_veloc = dev_veloc * mask_mag

        # --- Winkelteil ---
        dot = tf.reduce_sum(y_true * y_pred, axis=-1)
        cos = dot / (n_true * n_pred + eps)
        cos = tf.clip_by_value(cos, -1.0, 1.0)
        dev_angle = tf.acos(cos) / np.pi
        #dev_angle = dev_angle * mask_ang
        # --- Kombination ---
        loss = 1e-5 * dev_veloc + 0.4 * dev_angle
        return tf.reduce_mean(loss)

    @tf.function # Struktur: Bildähnlichkeit und MAE
    def my_loss_rho(self, y_true,y_pred):
        # Mixed-precision fix
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)
        # SSIM liefert Ähnlichkeit pro Bild → wir wollen daraus einen Loss machen
        ssim = tf.image.ssim(y_true, y_pred, max_val=1.0)
        ssim_loss = 1 - tf.reduce_mean(ssim)

        # MAE pro Pixel → mitteln
        mae = tf.reduce_mean(tf.abs(y_true - y_pred))

        # Kombination
        return mae + 0.5 * ssim_loss

    @tf.function
    def mse_var_norm(self, y_true,y_pred):
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)
        mse = tf.reduce_mean(tf.square(y_true - y_pred))
        w = tf.math.log(1 + tf.math.reduce_variance(y_true))
        w = tf.maximum(w,1e-6)
        loss = 0.3*mse
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
        return 1000*mse_rho+0.2*mse_grad_rho

    @tf.function
    def h1_norm_v(self, y_true, y_pred):
        # Mixed-precision fix
        y_true = tf.cast(y_true, tf.float32)
        y_pred = tf.cast(y_pred, tf.float32)

        # L2-Fehler der Werte
        mse_v = tf.reduce_mean(tf.square(y_true - y_pred))

        # Sobel-Gradienten
        sobel_true = tf.image.sobel_edges(y_true)  # (B,H,W,2,2,2)
        sobel_pred = tf.image.sobel_edges(y_pred)

        # L2-Fehler der Gradienten
        mse_grad = tf.reduce_mean(tf.square(sobel_true - sobel_pred))

        return 0.3*mse_v + 0.05*mse_grad

    def build_model_(self, load=False):
        if not load:
            # Wenn load=False, trainieren wir ein neues Modell

            self.model.compile(optimizer=self.optimizer,  loss={'out_rho': self.h1_norm_rho, 'out_v': self.h1_norm_v},
                               loss_weights=self.loss_weights,
                                jit_compile=False, metrics={"out_rho": self.h1_norm_rho, "out_v": self.h1_norm_v},
                               run_eagerly=False)


            early_stopping = tf.keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=8,
                min_delta=1e-4,
                restore_best_weights=True
            )
            reduce_lr = tf.keras.callbacks.ReduceLROnPlateau(monitor="val_loss", factor = 0.1, patience=2)

            self.model.summary(expand_nested=True)

            start_timer = time.perf_counter()

            # output verzeichnis
            save_dir = os.path.dirname(self.save_path)
            raw_dir = os.path.join(save_dir, "output")
            if not os.path.exists(raw_dir):
                os.makedirs(raw_dir)

            # Anpasse save_path für output
            raw_save_path = os.path.join(raw_dir, os.path.basename(self.save_path))


            self.history = self.model.fit(self.dataset_train, epochs=self.epoch,
                                          batch_size = self.batch_size,
                                          validation_data=self.dataset_val,
                                          callbacks=early_stopping
                                          )

            self.train_time = time.perf_counter() - start_timer
            print('Dauer Training:', self.train_time)

            #self.saved_model = tf.keras.models.save_model(self.model, raw_save_path + '.keras')
            self.saved_model = self.model.save(raw_save_path + '.keras')
            plot_model(self.model, to_file=raw_save_path + '_model.png',
                       show_shapes=True, expand_nested=True, show_layer_activations=True)

            plt.plot(self.history.history['loss'], label='Train')
            # funktioniert auch ohne val-daten
            if 'val_loss' in self.history.history:
                plt.plot(self.history.history['val_loss'], label='Eval')
            else:
                print("keine val daten")
            plt.ylim(0, max(plt.ylim()))
            plt.legend()
            plt.title('Loss')
            plt.savefig(raw_save_path + '_loss.png')
            plt.close()  

        else:
            # Wenn load=True, laden wir ein bestehendes Modell
            print('lade modell')
            if os.path.exists(self.load_path):
                self.model = tf.keras.models.load_model(self.load_path, compile=False)
                self.model.save("neu.keras")
            else:
                print(f"modell nicht gefunden: {self.load_path}")
                print("neues modell")

    def predict(self):
        # falls keine test-daten
        if self.dataset_test is None:
            print("keine test daten")
            return
        
        try:
            dataset = next(iter(self.dataset_test))
        except (StopIteration, TypeError):
            print("test dataset leer")
            return
            
        geo = dataset[0]['geometry']
        features = dataset[0]
        # labels haben Form (batch, H, W, C)
        rho_label = features['rho']  # (batch, 1, H, W, 1)
        v_label = features['v']  # (batch, 1, H, W, 2)

        # in Plot-Format bringen
        rho_label = tf.transpose(rho_label, [0, 1, 4, 2, 3])  # (B,1,C,H,W)
        v_label = tf.transpose(v_label, [0, 1, 4, 2, 3])

        labels = (rho_label, v_label, geo)
        rho = dataset[0]['rho']
        v = dataset[0]['v']

        result_steps = self.pred_steps - self.dataset.feature_time_steps# Schritte für Vorhersage - Inputzeitschritte

        data = {
              'rho': rho[:,:self.dataset.feature_time_steps,:,:],
               'v': v[:,:self.dataset.feature_time_steps,:,:],
              'geometry': geo[:,:self.dataset.feature_time_steps,:,:]
               }  # wird für die iterative Vorhersage genutzt, übernimmt immer nur die letzten 5 Zeitschritte
        data_temp = data # speichert alle Zeitschritte
        start_timer = time.perf_counter()

        for step in range(result_steps - self.initial_step_pred):
              predictions = self.model.predict(data,batch_size=self.batch_size)

              new_rho = predictions[0][::, tf.newaxis, ::, ::, ::] # zusätzliche Zeitdim. für Outputdaten erzeugen
              rho_temp = tf.concat((data_temp['rho'], new_rho), axis=1)
              new_rho = tf.concat((data['rho'], new_rho), axis=1) # an Inputdaten anhängen

              new_v = predictions[1][::, tf.newaxis, ::, ::, ::]
              v_temp = tf.concat((data_temp['v'], new_v), axis=1)
              new_v = tf.concat((data['v'], new_v), axis=1)


              data = {
                  'rho': new_rho[:,-self.dataset.feature_time_steps:,:,:], # letzten feature-time_steps-Zeitschritte verwenden
                  'v': new_v[:,-self.dataset.feature_time_steps:,:,:],
                  'geometry': geo[:,-self.dataset.feature_time_steps:,:,:]
              }

              data_temp = {'rho': rho_temp,
                         'v': v_temp,
                         'geometry': geo}


        self.pred_time = time.perf_counter()- start_timer
        print('Dauer Vorhersage:', self.pred_time)

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
