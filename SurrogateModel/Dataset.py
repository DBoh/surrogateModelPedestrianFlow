import numpy as np
import pandas as pd
import csv
from matplotlib import pyplot as plt
from matplotlib import colors
import tensorflow as tf


class Dataset:
    def __init__(self, files, feature_time_steps, pred_steps, label_time_steps, size_geometry, batch_size,
                 start_point=0,
                 time_stride=1):
        self.files = files
        self.feature_time_steps = feature_time_steps
        self.pred_steps = pred_steps
        self.label_time_steps = label_time_steps
        self.start_point = start_point
        self.time_stride = time_stride
        self.size_geometry = size_geometry
        self.batch_size = batch_size
        self.dataset_train = None
        self.x_train = None
        self.y_rho_train = None
        self.y_v_train = None
        self.analyse_param_train = None
        self.dataset_test = None
        self.x_test = None
        self.y_rho_test = None
        self.y_v_test = None
        self.analyse_param_test = None
        self.dataset_val = None
        self.x_val = None
        self.y_rho_val = None
        self.y_v_val = None
        self.analyse_param_val = None
        self.predictions = None
        self.labels = None
        self.result_steps = None

    def load_data(self):
        self.read_tfrecord()

## VISUALIZATION OF DATA

    def show_label(self):
        data = next(iter(self.dataset_train))

        label = data[1][0]
        label = np.transpose(label, [0, 1, 4, 2, 3])
        plt.imshow(label[0, 5, 0], origin='lower')
        plt.show()

        features = data[1][1]
        features = np.transpose(features, [0, 1, 4, 2, 3])
        X, Y = np.meshgrid(np.arange(0, self.size_geometry[0], 1), np.arange(0, self.size_geometry[1], 1))
        plt.quiver(X, Y, features[0, 5, 0], features[0, 5, 1])
        plt.show()

    def show_feature(self):
        data = next(iter(self.dataset_train))

        features = data[0]['rho']
        features = np.transpose(features, [0, 1, 4, 2, 3])
        plt.imshow(features[0, 5, 0], origin='lower')
        plt.show()

        features = data[0]['v']
        features = np.transpose(features, [0, 1, 4, 2, 3])
        X, Y = np.meshgrid(np.arange(0, self.size_geometry[0], 1), np.arange(0, self.size_geometry[1], 1))
        plt.quiver(X, Y, features[0, 5, 0], features[0, 5, 1])
        plt.show()

    def show_geom(self, save_path):
        data = next(iter(self.dataset_train))

        geo = np.transpose(data[0]['geometry'], [0, 1, 4, 2, 3])
        fig, axs = plt.subplots(1, 2, sharex=True)
        axs[0].imshow(geo[0, 0, 0], origin='lower')
        axs[1].imshow(geo[0, 0, 1], origin='lower')
        plt.title("Geometry")
        if save_path is not None:
            plt.savefig(save_path + f'_geo.png')
        plt.show()

    def show_all(self):
        data = next(iter(self.dataset_train))
        geo = np.transpose(data[0]['geometry'], [0, 1, 4, 2, 3])
        fig, ax = plt.subplots()
        im = ax.imshow(geo[0, 0, 0], origin='lower')
        ax.set_title('Geometry: FRC')
        fig.colorbar(im, ax=ax)
        plt.show()
        plt.imshow(geo[0, 0, 1], origin='lower')
        plt.title('Geometry: SDF')
        plt.show()

        features = data[0]['rho']
        features = np.transpose(features, [0, 1, 4, 2, 3])
        plt.imshow(features[0, 5, 0], origin='lower')
        plt.title('Dichte Rho: t = 5')
        plt.show()

        features = data[0]['v']
        features = np.transpose(features, [0, 1, 4, 2, 3])
        X, Y = np.meshgrid(np.arange(0, self.size_geometry[0], 1), np.arange(0, self.size_geometry[1], 1))
        plt.quiver(X, Y, features[0, 5, 0], features[0, 5, 1])
        plt.title('Geschwindigkeit v: t = 5')
        plt.show()

    ################################################
    # READ DATA AUS TFRECORD-DATEIEN
    ################################################

    def read_tfrecord(self):
        """
        Read data from a TFRecord file and return the parsed dataset.
        :type self: object
        :param self:
        :param take: The number of records to take from the TFRecord file.
        :param files: The list of the TFRecord files. Defaults to 'data.tfrecord'
        :param batch_size: The batch size for the dataset. Defaults to 32.
        :return: The parsed dataset split into x and y, where x is the features and y is the label within x and y.
        """
        if not len(self.files) == 3:
            raise ValueError('there must be three file sets: test, training, validation')

        take_exp = [-1, -1, -1]  # -1 = alle Samples laden
        
        shuffle = [False, True, False] # nur Trainingsdaten werden geshuffelt, damit er nicht die zeitliche Reihenfolge der Samples lernt

        for idx in range(len(self.files)):

            num_parallel_calls = tf.data.experimental.AUTOTUNE

            if isinstance(self.files[idx], list):
                if shuffle[idx]:
                    files = tf.random.shuffle(self.files[idx])
                else:
                    files = self.files[idx]
            else:
                # fallback: verzeichnispfad
                import glob
                files = glob.glob(self.files[idx] + '*.tfrecord')
                if shuffle[idx]:
                    files = tf.random.shuffle(files)
                
            file_paths = tf.constant(files, dtype=tf.string)
            shards = tf.data.Dataset.from_tensor_slices(file_paths)

            dataset = shards.interleave(tf.data.TFRecordDataset, num_parallel_calls=num_parallel_calls)
            if take_exp[idx] > 0:
                dataset = dataset.take(take_exp[idx])
            
            dataset_size = len(files) * 5  # 5 samples pro tfrecord

            dataset = dataset.map(parse_tfrecord_fn, num_parallel_calls=num_parallel_calls)
            datasets = []
            if idx == 0: # test
                start_times = 1
                for start in range(start_times): # einlesen der gesamten Zeitreihe für Testdaten
                    ds = dataset.map(lambda x, y, z: preprocess(x, y, z,
                                                                feature_time_steps=self.pred_steps-self.label_time_steps,
                                                                label_time_steps=self.label_time_steps,
                                                                start_time=start,
                                                                size_geometry=self.size_geometry,
                                                                time_stride=self.time_stride
                                                                ),
                                     num_parallel_calls=num_parallel_calls)
                    datasets.append(ds)
            else: # training 1, validierung 2
                start_times = self.pred_steps-self.feature_time_steps-self.label_time_steps
                for start in range(start_times): # sliding window für Trainings- und Validierungsdaten
                  ds = dataset.map(lambda x, y, z: preprocess(x, y, z,
                                                             feature_time_steps=self.feature_time_steps,
                                                             label_time_steps=self.label_time_steps,
                                                             start_time=start,
                                                             size_geometry=self.size_geometry,
                                                             time_stride=self.time_stride),
                                  num_parallel_calls=num_parallel_calls)
                  datasets.append(ds)

            dataset = datasets[0]
            for ds in datasets[1:]:
                dataset = dataset.concatenate(ds)
            if shuffle[idx]:
                buffer_size = dataset_size
                dataset = dataset.shuffle(buffer_size=250)
                print(f"shuffle buffer: {buffer_size}")
            dataset = dataset.batch(self.batch_size, drop_remainder=True)
            if idx == 0:
                self.dataset_test = dataset.map(lambda x, y_rho, y_v, z: (x, (y_rho, y_v)))  
                self.x_test = dataset.map(lambda x, y_rho, y_v, z: x)
                self.y_rho_test = dataset.map(lambda x, y_rho, y_v, z: y_rho)
                self.y_v_test = dataset.map(lambda x, y_rho, y_v, z: y_v)
                self.analysis_params = dataset.map(lambda x, y_rho, y_v, z: z)
                print(f"Testdaten geladen")
            elif idx == 1:
                self.dataset_train = dataset.map(lambda x, y_rho, y_v, z: (x, (y_rho, y_v)))  
                self.x_train = dataset.map(lambda x, y_rho, y_v, z: x)
                self.y_rho_train = dataset.map(lambda x, y_rho, y_v, z: y_rho)
                self.y_v_train = dataset.map(lambda x, y_rho, y_v, z: y_v)
                self.analysis_params = dataset.map(lambda x, y_rho, y_v, z: z)
                print(f"Trainingsdaten geladen")
            elif idx == 2:
                self.dataset_val = dataset.map(lambda x, y_rho, y_v, z: (x, (y_rho, y_v)))  
                self.x_val = dataset.map(lambda x, y_rho, y_v, z: x)
                self.y_rho_val = dataset.map(lambda x, y_rho, y_v, z: y_rho)
                self.y_v_val = dataset.map(lambda x, y_rho, y_v, z: y_v)
                self.analysis_params = dataset.map(lambda x, y_rho, y_v, z: z)
                print(f"Validierungsdaten geladen")

        return self

    ### VISUALIZE RESULTS: wird bei model.predict() am Ende aufgerufen

    def print_label_predict_rho_v(self, save_path=None):
        fontsize = 5
        time_steps = self.result_steps  # samples nach stride, nicht raw frames

        norm = colors.Normalize(vmin=0, vmax=0.25)

        for id in range(self.predictions[0].shape[0]):
            data = [[], [], []]
            for idx in range(time_steps):
                images = []
                fig, axs = plt.subplots(2, 3, sharex=True, sharey=True)
                fig.suptitle(f'Timestep: {idx * self.time_stride}', fontsize=8)
                label = self.labels[0][id, idx, 0]
                images.append(axs[0, 0].imshow(label, origin='lower'))
                axs[0, 0].set_title("Label rho", fontsize=fontsize)

                prediction = self.predictions[0][id, idx, 0]

                images.append(axs[0, 1].imshow(prediction, origin='lower'))
                axs[0, 1].set_title("Prediction rho", fontsize=fontsize)

                mse = np.subtract(label, prediction) ** 2
                images.append(axs[0, 2].imshow(np.sqrt(mse), origin='lower'))
                axs[0, 2].set_title(f"RMSE Error rho: {np.sqrt(np.mean(np.sqrt(mse))):.2f}", fontsize=fontsize)
                data[0].append(np.sqrt(np.mean(mse)))

                X, Y = np.meshgrid(np.arange(0, self.size_geometry[0], 1), np.arange(0, self.size_geometry[1], 1))

                ul, vl = self.labels[1][id, idx, 0], self.labels[1][id, idx, 1]
                axs[1, 0].quiver(X, Y, ul, vl, cmap='viridis')
                axs[1, 0].set_title("Label v", fontsize=fontsize)

                up, vp = self.predictions[1][id, idx, 0], self.predictions[1][id, idx, 1]
                axs[1, 1].quiver(X, Y, up, vp, cmap = 'viridis')
                axs[1, 1].set_title("Prediction v", fontsize=fontsize)

                u, v = np.subtract(ul, up), np.subtract(vl, vp)
                axs[1, 2].quiver(X, Y, u, v, cmap = 'viridis')
                axs[1, 2].set_title(f"Error v")

                data[1].append(np.sqrt(np.mean(np.add(u ** 2, v ** 2))))  
                data[2].append(np.sqrt(np.mean(
                    np.subtract(np.sqrt(ul ** 2 + vl ** 2), np.sqrt(up ** 2 + vp ** 2)) ** 2)))  
                #norm = colors.Normalize(vmin=0, vmax=np.max(label))
                for im in images:
                    im.set_norm(norm)
                plt.colorbar(images[0], ax=axs, orientation='vertical')
                if save_path is not None:
                    plt.savefig(save_path + f'_BSP{id}_t{idx * self.time_stride}.png')
                plt.close(fig)

                df = pd.DataFrame(label)
                df.to_csv(save_path + f'_label_BSP{id}_t{idx * self.time_stride}.csv')
                df = pd.DataFrame(prediction)
                df.to_csv(save_path + f'_prediction_BSP{id}_t{idx * self.time_stride}.csv')



            with (open(save_path + f'_BSP{id}' + f'_prediction_rmse.csv', 'w', newline='') as file):
                fieldnames = ['Timestep', 'RMSE Rho', 'RMSE V', 'RMSE Phi']
                writer = csv.writer(file)
                writer.writerow(range(0, self.result_steps * self.time_stride, self.time_stride))
                writer.writerow(data[0])
                writer.writerow(data[1])
                writer.writerow(data[2])

########## AUXILIARY FUNCTIONS

def parse_tfrecord_fn(example):
    feature_description = {
        'char_density': tf.io.FixedLenFeature([], tf.int64),
        'char_velocity': tf.io.FixedLenFeature([], tf.float32),
        'start_persons': tf.io.FixedLenFeature([], tf.int64),
        'exit_points': tf.io.FixedLenFeature([], tf.string),
        'geometry': tf.io.FixedLenFeature([], tf.string),
        'position': tf.io.FixedLenFeature([], tf.string),
        'time': tf.io.FixedLenFeature([], tf.float32),
        'label': tf.io.FixedLenFeature([], tf.string),
        'analysis_params': tf.io.FixedLenFeature([], tf.string)
    }
    example = tf.io.parse_example(example, feature_description)

    char_density = example['char_density']
    char_velocity = example['char_velocity']
    start_persons = example['start_persons']
    exit_points = tf.io.parse_tensor(example['exit_points'], out_type=tf.int32)
    geometry = tf.io.parse_tensor(example['geometry'], out_type=tf.int32)
    label = tf.io.parse_tensor(example['label'], out_type=tf.float64)
    analysis_params = tf.py_function(lambda x: x.numpy().decode('utf-8'), [example['analysis_params']], tf.string)
    param = tf.stack([float(char_density), char_velocity, float(start_persons)])

    selected_features = {
        'parameter': param,
        'exit_points': exit_points,
        'geometry': geometry,
    }
    return selected_features, label, analysis_params

def random_sample(data, num_samples):
    indices = tf.range(tf.shape(data)[0])  
    chosen_indices = tf.random.shuffle(indices)[:num_samples]  

    return tf.gather(data, chosen_indices)  

def exp_dim(data, dim):
    return tf.tile(tf.expand_dims(data, axis=0), multiples=[dim, 1, 1, 1])


def preprocess(features, label, analyse, feature_time_steps, label_time_steps, size_geometry, start_time, time_stride):
    import sys
    from pathlib import Path
    sys.path.append(str(Path(__file__).parent.parent))
    from generate.config import TimeConfig
    
    if label_time_steps > TimeConfig.TFRECORD_MAX_STEPS:
        raise ValueError(f"Der Startpunkt liegt zu weit hinten. {label_time_steps} > {TimeConfig.TFRECORD_MAX_STEPS}.")
# hier werden die entsprechenden Zeitfenster der Zeitreihen eingelesen: Für Inputdaten
    feature_rho, feature_v = change_size_startfield(label, feature_time_steps=feature_time_steps,
                                                    start_time=start_time,
                                                    time_stride=time_stride)
    # hier werden die entsprechenden Zeitfenster der Zeitreihen eingelesen: Für Outputdaten
    label_rho, label_v, label_phi = change_size_label(label, label_time_steps, feature_time_steps,
                                           start_time,
                                           time_stride=time_stride)

    if feature_time_steps == 0:  
        feature_rho = tf.convert_to_tensor(tf.transpose(feature_rho, perm=[1, 2, 0]))
        feature_v = tf.convert_to_tensor(tf.transpose(feature_v, perm=[1, 2, 0]))
        label_rho = tf.convert_to_tensor(tf.transpose(label_rho, perm=[0, 2, 3, 1]))
        label_v = tf.convert_to_tensor(tf.transpose(label_v, perm=[0, 2, 3, 1]))
        label_rho = label_rho[-1,:,:,:]
        label_v = label_v[-1,:,:,:]
        geometry = tf.reshape(tf.convert_to_tensor(tf.cast(tf.transpose(features['geometry'], perm=[1, 2, 0]), dtype=tf.float32),
                                 dtype=tf.float32), shape=(*size_geometry, 2))

    else:
        feature_rho = tf.convert_to_tensor(tf.transpose(feature_rho,perm=[0,2,3,1]))
        feature_v = tf.convert_to_tensor(tf.transpose(feature_v, perm=[0, 2, 3, 1]))
        if label_time_steps - feature_time_steps > 0:
             label_time_steps = label_time_steps - feature_time_steps

        label_rho = tf.convert_to_tensor(tf.transpose(label_rho,perm=[0,2,3,1]))
        label_rho = label_rho[-1,:,:,:] # Zeit-Dimension wird für Output-Daten entfernt

        label_v = tf.convert_to_tensor(tf.transpose(label_v,perm=[0,2,3,1]))
        label_v = label_v[-1,:,:,:]
        geometry = tf.reshape(tf.convert_to_tensor(
            tf.cast(tf.transpose(exp_dim(features['geometry'], feature_time_steps // time_stride), perm=[0, 2, 3, 1]), dtype=tf.float32),
            dtype=tf.float32), shape=(feature_time_steps // time_stride, *size_geometry, 2))
        label_phi = tf.reshape(tf.convert_to_tensor(tf.cast(tf.transpose(exp_dim(label_phi[0,::,::,::], feature_time_steps // time_stride),
                                                                          perm=[0, 2, 3, 1]), dtype=tf.float32),
            dtype=tf.float32), shape=(feature_time_steps // time_stride, *size_geometry, 1))

        geometry = tf.reshape(tf.convert_to_tensor(
            tf.cast(geometry, dtype=tf.float32),
            dtype=tf.float32), shape=(feature_time_steps // time_stride, *size_geometry, 2))

    feature = {
        'rho': feature_rho,
        'v': feature_v,
        'geometry': geometry
    }
    label = {
        "rho": label_rho,
        "v": label_v
    }
    return feature, label_rho, label_v, analyse

# Funktionen zum Einlesen der richtigen Zeitfenster aus den Zeitreihen: für Input und Output
def change_size_startfield(data, feature_time_steps, start_time=0, time_stride=1,
                            size_geometry=(64, 64)):
    if feature_time_steps == 0:
        data = tf.convert_to_tensor([data[0, 0:size_geometry[0], 0:size_geometry[1]],
                                     data[1, :, ::2],
                                     data[1, :, 1::2]
                                     ])
        return data[:1], data[1:3], data[::, 3:4]

    data = tf.stack(
        [data[start_time:(start_time + feature_time_steps):time_stride, 0, 0:size_geometry[0], 0:size_geometry[1]],
         data[start_time:(start_time + feature_time_steps):time_stride, 1, :, ::2],
         data[start_time:(start_time + feature_time_steps):time_stride, 1, :, 1::2]
         ], axis=1)
    return data[::, :1], data[::, 1:3]

# label = Outputdaten
def change_size_label(data, label_time_steps, feature_time_steps, start_time=0, time_stride=1,
                      size_geometry=(64, 64)):
    data = tf.stack([data[start_time + feature_time_steps:(start_time + feature_time_steps+ label_time_steps):time_stride, 0, 0:size_geometry[0], 0:size_geometry[1]],
                     data[start_time + feature_time_steps:(start_time + feature_time_steps+ label_time_steps):time_stride, 1, :, ::2],
                     data[start_time + feature_time_steps:(start_time  + feature_time_steps+ label_time_steps):time_stride, 1, :, 1::2],
                     data[start_time + feature_time_steps:(start_time +  feature_time_steps +label_time_steps):time_stride, 2, 0:size_geometry[0], 0:size_geometry[1]]
                     ], axis=1)
    label_phi = data[:, 3::4]
    label_rho = data[::,:1]
    label_v = data[::,1:3]

    return label_rho,label_v, label_phi
