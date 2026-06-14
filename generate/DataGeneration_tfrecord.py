import os
import random

import tensorflow as tf


def write_features_tfrecord(data, tfr_path):
    """
    Writes the data to a TensorFlow Record (TFRecord) file.
    :param data:  The features to be written to the TFRecord file.
    :param tfr_path: The path to the TFRecord file.
    :return: None
    """
    # verzeichnis erstellen
    tfr_dir = os.path.dirname(tfr_path)
    os.makedirs(tfr_dir, exist_ok=True)

    # temp datei
    tfr_path_temp = tfr_path + ".temp"

    if os.path.exists(tfr_path):
        dataset = tf.data.TFRecordDataset([tfr_path])
    else:
        dataset = []
    
    try:
        with tf.io.TFRecordWriter(tfr_path_temp) as writer:
            for record in dataset:
                writer.write(record.numpy())

            for d in data:
                # check daten
                if len(d) < 12:
                    raise ValueError(f"Unvollständige Daten: Erwartet 12 Elemente, erhalten {len(d)}")
                
                feature = {
                    'char_density': tf.train.Feature(int64_list=tf.train.Int64List(value=[d[0]])),
                    'char_velocity': tf.train.Feature(float_list=tf.train.FloatList(value=[d[1]])),
                    'start_persons': tf.train.Feature(int64_list=tf.train.Int64List(value=[d[2]])),
                    'fundamentaldiagramm': tf.train.Feature(int64_list=tf.train.Int64List(value=[d[3]])),
                    'exit_points': tf.train.Feature(bytes_list=tf.train.BytesList(value=[tf.io.serialize_tensor(d[4]).numpy()])),
                    'geometry': tf.train.Feature(bytes_list=tf.train.BytesList(value=[tf.io.serialize_tensor(d[7]).numpy()])),
                    'position': tf.train.Feature(bytes_list=tf.train.BytesList(value=[tf.io.serialize_tensor(d[8]).numpy()])),
                    'time': tf.train.Feature(float_list=tf.train.FloatList(value=[d[9]])),
                    'label': tf.train.Feature(bytes_list=tf.train.BytesList(value=[tf.io.serialize_tensor(d[10]).numpy()])),
                    'analysis_params': tf.train.Feature(bytes_list=tf.train.BytesList(value=[str(d[11]).encode('utf-8')]))
                }

                example = tf.train.Example(features=tf.train.Features(feature=feature))
                writer.write(example.SerializeToString())

        # alte durch neue ersetzen
        if os.path.exists(tfr_path):
            os.remove(tfr_path)
        os.rename(tfr_path_temp, tfr_path)
    except (IOError, OSError, tf.errors.OpError) as e:
        if os.path.exists(tfr_path_temp):
            try:
                os.remove(tfr_path_temp)
            except OSError:
                pass
        raise RuntimeError(f"Fehler beim Schreiben der TFRecord-Datei: {e}")
    except Exception as e:
        if os.path.exists(tfr_path_temp):
            try:
                os.remove(tfr_path_temp)
            except OSError:
                pass
        raise RuntimeError(f"Unerwarteter Fehler beim Schreiben der TFRecord-Datei: {e}")


def parse_tfrecord_fn(example):
    """
    Parses a TFRecord example and returns selected features and labels.
    :param example: A single TFRecord example.
    :return Tuple[Dict[str, tf.Tensor], tf.Tensor]: A tuple containing a dictionary of selected features and the label.
    """
    feature_description = {
        'char_density': tf.io.FixedLenFeature([], tf.int64),
        'char_velocity': tf.io.FixedLenFeature([], tf.float32),
        'start_persons': tf.io.FixedLenFeature([], tf.int64),
        'fundamentaldiagramm': tf.io.FixedLenFeature([], tf.int64),
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
    fundamentaldiagramm = example['fundamentaldiagramm']
    exit_points = tf.io.parse_tensor(example['exit_points'], out_type=tf.int32)
    geometry = tf.io.parse_tensor(example['geometry'], out_type=tf.int32)
    position = tf.io.parse_tensor(example['position'], out_type=tf.string)
    time = example['time']
    label = tf.io.parse_tensor(example['label'], out_type=tf.float64)
    analysis_params = tf.py_function(lambda x: x.numpy().decode('utf-8'), [example['analysis_params']], tf.string)
    param = tf.stack([float(char_density), char_velocity, float(start_persons)])

    selected_features = {
        'parameter': param,
        'exit_points': exit_points,
        'geometry': geometry,
    }
    return selected_features, label, analysis_params


def minmax_scaling(feature, label_rho, label_v, label_phi, analyse):
    # Min-Max werte wurden von Hand aus den Daten gelesen -> Funktion Analyse beinhaltet Informationen
    # Für room3
    range_rho = tf.constant([0.0, 1.0], dtype=tf.float64) # geändert 27.05.2026 von 0.25 auf 1.0
    range_v = tf.constant([0.0, 1.34], dtype=tf.float64)
    range_phi = tf.constant([0.0, 1.0], dtype=tf.float64)
    range_norm = tf.constant([0.0, 1.0], dtype=tf.float64)


    feature['rho'] = scaling_minmax_dataset(feature['rho'], range_rho, range_norm)
    feature['v'] = scaling_minmax_dataset(feature['v'], range_v, range_norm)
    feature['phi'] = scaling_minmax_dataset(feature['phi'], range_phi, range_norm)

    label_rho = scaling_minmax_dataset(label_rho, range_rho, range_norm)
    label_v = scaling_minmax_dataset(label_v, range_v, range_norm)
    label_phi = scaling_minmax_dataset(label_phi, range_phi, range_norm)

    return feature, label_rho, label_v, label_phi, analyse


def scaling_minmax_dataset(data, current_range, normed_range):
    scaled_data = ((data - current_range[0])*(normed_range[1] - normed_range[0]) /
                   (current_range[1] - current_range[0])) + normed_range[0]
    return scaled_data


def change_size_startfield(data, number_steps, start_time=0, size_geometry=(40, 40), time_stride=1, time_stride_random=False):
    if number_steps == 0:
        data = tf.convert_to_tensor([data[0, 0:size_geometry[0], 0:size_geometry[1]],
                                     data[1, :, ::2],
                                     data[1, :, 1::2],
                                     data[2, 0:size_geometry[0], 0:size_geometry[1]]
                                     ])
        return data[:1], data[1:3], data[::, 3:4]
    if time_stride_random:
        first = data[0]
        last = data[-1]
        random_elem = random.sample(data[1:-1], number_steps-2)
        data = tf.stack([first, random_elem, last])
        time_stride = 1
    data = tf.stack([
        data[start_time:(start_time+number_steps):time_stride, 0, 0:size_geometry[0], 0:size_geometry[1]],
        data[start_time:(start_time+number_steps):time_stride, 1, :, ::2],
        data[start_time:(start_time+number_steps):time_stride, 1, :, 1::2],
        data[start_time:(start_time+number_steps):time_stride, 2, 0:size_geometry[0], 0:size_geometry[1]]
    ], axis=1)
    return data[::, :1], data[::, 1:3], data[::, 3:4]


def change_size_label(data, time_steps, start_time=0, size_geometry=(40, 40), time_stride=1, time_stride_random=False):
    if time_stride_random:
        first = data[0]
        last = data[-1]
        random_elem = random.sample(data[1:-1], time_steps-2)
        data = tf.stack([first, random_elem, last])
        time_stride = 1

    data = tf.stack([
        data[start_time:(start_time+time_steps):time_stride, 0, 0:size_geometry[0], 0:size_geometry[1]],
        data[start_time:(start_time+time_steps):time_stride, 1, :, ::2],
        data[start_time:(start_time+time_steps):time_stride, 1, :, 1::2],
        data[start_time:(start_time+time_steps):time_stride, 2, 0:size_geometry[0], 0:size_geometry[1]]
    ], axis=1)
    return data[::, :1], data[::, 1:3], data[::, 3::4]


def flip_label_v(label):
    return tf.reverse(label, axis=[2])


def exp_dim(data, dim):
    return tf.tile(tf.expand_dims(data, axis=0), multiples=[dim, 1, 1, 1])


def preprocess(features, label, analyse, feature_time_steps, label_time_steps, size_geometry, start_time, time_stride, time_stride_random):
    if label_time_steps > 500:
        raise ValueError(f"Der Startpunkt liegt zu weit hinten. {label_time_steps} > 500.")

    feature_rho, feature_v, feature_phi = change_size_startfield(label,
                                                                 number_steps=feature_time_steps,
                                                                 start_time=start_time,
                                                                 size_geometry=size_geometry,
                                                                 time_stride=time_stride,
                                                                 time_stride_random=time_stride_random)

    if label_time_steps - feature_time_steps > 0:
        start_time = start_time + feature_time_steps
    label_rho, label_v, label_phi = change_size_label(label,
                                                      time_steps=label_time_steps,
                                                      start_time=start_time,
                                                      size_geometry=size_geometry,
                                                      time_stride=time_stride,
                                                      time_stride_random=time_stride_random)

    if feature_time_steps == 0: #für convlstm_unet
        feature_rho = tf.convert_to_tensor(tf.transpose(feature_rho, perm=[1, 2, 0]))
        feature_v = tf.convert_to_tensor(tf.transpose(feature_v, perm=[1, 2, 0]))
        feature_phi = tf.convert_to_tensor(tf.transpose(feature_phi, perm=[1, 2, 0]))
        label_rho = tf.convert_to_tensor(tf.transpose(label_rho, perm=[0, 2, 3, 1]))
        label_v = tf.convert_to_tensor(tf.transpose(label_v, perm=[0, 2, 3, 1]))
        label_phi = tf.convert_to_tensor(tf.transpose(label_phi, perm=[0, 2, 3, 1]))

        geometry = tf.reshape(
            tf.convert_to_tensor(tf.cast(tf.transpose(features['geometry'], perm=[1, 2, 0]), dtype=tf.float32),
                                 dtype=tf.float32), shape=(*size_geometry, 2))
        exit_points = tf.reshape(tf.convert_to_tensor(tf.transpose(features['exit_points'], perm=[1, 0])), shape=(4, 5))

    else: # convlstm
        feature_rho = tf.reshape(tf.convert_to_tensor(tf.transpose(feature_rho, perm=[0, 2, 3, 1])), shape=(feature_time_steps // time_stride, *size_geometry, 1))
        feature_v = tf.reshape(tf.convert_to_tensor(tf.transpose(feature_v, perm=[0, 2, 3, 1])), shape=(feature_time_steps // time_stride, *size_geometry, 2))
        feature_phi = tf.reshape(tf.convert_to_tensor(tf.transpose(feature_phi, perm=[0, 2, 3, 1])), shape=(feature_time_steps // time_stride, *size_geometry, 1))
        if label_time_steps - feature_time_steps > 0:
            label_time_steps = label_time_steps - feature_time_steps
        label_rho = tf.reshape(tf.convert_to_tensor(tf.transpose(label_rho, perm=[0, 2, 3, 1])), shape=(label_time_steps // time_stride, *size_geometry, 1))
        label_v = tf.reshape(tf.convert_to_tensor(tf.transpose(label_v, perm=[0, 2, 3, 1])), shape=(label_time_steps // time_stride, *size_geometry, 2))
        label_phi = tf.reshape(tf.convert_to_tensor(tf.transpose(label_phi, perm=[0, 2, 3, 1])), shape=(label_time_steps // time_stride, *size_geometry, 1))

        geometry = tf.reshape(tf.convert_to_tensor(
            tf.cast(tf.transpose(exp_dim(features['geometry'], feature_time_steps // time_stride), perm=[0, 2, 3, 1]), dtype=tf.float32),
            dtype=tf.float32), shape=(feature_time_steps // time_stride, *size_geometry, 2))
        exit_points = tf.reshape(tf.convert_to_tensor(tf.transpose(features['exit_points'], perm=[1, 0])), shape=(4, 5))

    feature = {
        'parameter': features['parameter'],
        'exit_points': exit_points,
        'geometry': geometry,
        'rho': feature_rho,
        'v': feature_v,
        'phi': feature_phi
    }
    label = {
        "rho": label_rho,
        "v": label_v,
        "phi": label_phi
    }
    return feature, label_rho, label_v, label_phi, analyse


def split_v(label_v):
    label_vx = label_v[::, ::, ::, ::, :1]
    label_vy = label_v[::, ::, ::, ::, 1:2]

    return label_vx, label_vy

def read_tfrecord(take_exp, batch_size, shuffle, files, feature_steps, label_steps, size_geometry, start_point, time_stride, time_stride_random):
    dataset = tf.data.TFRecordDataset(files)
    dataset = dataset.map(parse_tfrecord_fn, num_parallel_calls=tf.data.AUTOTUNE)

    # Wir gehen davon aus, dass parse_tfrecord_fn folgendes liefert:
    # features, label, analyse
    # wobei "label" die Zeitreihe enthält: (T, H, W, C)

    def create_windows(features, label, analyse):
        X, Y = make_sliding_windows(label, feature_steps, label_steps)
        # Wir erzeugen ein Dataset pro Beispiel
        return tf.data.Dataset.from_tensor_slices((X, Y, analyse))

    dataset = dataset.flat_map(create_windows)
    dataset = dataset.map(lambda x, y, z: preprocess(x, y, z,
                                                    feature_time_steps=feature_time_steps + start_point,
                                                    label_time_steps=label_time_steps + start_point,
                                                    start_time=start_point,
                                                    size_geometry=(size_geometry[0], size_geometry[1]),
                                                    time_stride=time_stride,
                                                    time_stride_random=time_stride_random))
    dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: minmax_scaling(x, y_rho, y_v, y_phi, z))
    if shuffle:
        dataset = dataset.shuffle(buffer_size=take_exp)
    dataset = dataset.batch(batch_size, drop_remainder=True)

    return dataset

def make_sliding_windows(sequence, feature_steps, label_steps):
    """
    sequence: Tensor der Form (T, H, W, C)
    feature_steps: z.B. 5
    label_steps: z.B. 1
    """
    total = tf.shape(sequence)[0]
    windows = total - feature_steps - label_steps + 1

    X = []
    Y = []

    for t in tf.range(windows):
        x_t = sequence[t : t + feature_steps]
        y_t = sequence[t + feature_steps : t + feature_steps + label_steps]
        X.append(x_t)
        Y.append(y_t)

    return tf.stack(X), tf.stack(Y)

def read_tfrecord_old(take_exp, batch_size, shuffle, files, feature_time_steps, label_time_steps, size_geometry, start_point, time_stride, time_stride_random):
    """
    Read data from a TFRecord file and return the parsed dataset.
    :param take: The number of records to take from the TFRecord file.
    :param files: The list of the TFRecord files. Defaults to 'data.tfrecord'
    :param batch_size: The batch size for the dataset. Defaults to 32.
    :return: The parsed dataset split into x and y, where x is the features and y is the label within x and y.
    """
    if shuffle:
        files = tf.random.shuffle(files)
    shards = tf.data.Dataset.from_tensor_slices(files)
    dataset = shards.interleave(tf.data.TFRecordDataset)
    if take_exp > 0:
        dataset = dataset.take(take_exp)
    dataset_size = dataset.reduce(tf.cast(0, tf.int64), lambda x, _: x + 1)
    dataset = dataset.map(parse_tfrecord_fn, num_parallel_calls=tf.data.AUTOTUNE)
    dataset = dataset.map(lambda x, y, z: preprocess(x, y, z,
                                                     feature_time_steps=feature_time_steps+start_point,
                                                     label_time_steps=label_time_steps+start_point,
                                                     start_time=start_point,
                                                     size_geometry=(size_geometry[0], size_geometry[1]),
                                                     time_stride=time_stride,
                                                     time_stride_random=time_stride_random))
    dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: minmax_scaling(x, y_rho, y_v, y_phi, z))
    if shuffle:
        dataset = dataset.shuffle(buffer_size=take_exp)
    dataset = dataset.batch(batch_size, drop_remainder=True)
    #dataset = dataset.prefetch(buffer_size=tf.data.experimental.AUTOTUNE)


    dataset_rho_v_phi = dataset.map(lambda x, y_rho, y_v, y_phi, z: (x, (y_rho, y_v, y_phi)))
    dataset_rho = dataset.map(lambda x, y_rho, y_v, y_phi, z: (x, y_rho))
    dataset_rho_v = dataset.map(lambda x, y_rho, y_v, y_phi, z: (x, (y_rho, y_v)))
    dataset_rho_phi = dataset.map(lambda x, y_rho, y_v, y_phi, z: ((x, y_rho, y_phi)))

    dataset_rho_vx_vy_phi = dataset.map(lambda x, y_rho, y_v, y_phi, z: (x, (y_rho, *split_v(y_v), y_phi)))
    dataset_rho_vx_vy = dataset.map(lambda x, y_rho, y_v, y_phi, z: (x, (y_rho, *split_v(y_v))))

    x_dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: x)
    y_rho_dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: y_rho)
    y_v_dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: y_v)
    y_phi_dataset = dataset.map(lambda x, y_rho, y_v, y_phi, z: y_phi)

    analysis_params = dataset.map(lambda x, y_rho, y_v, y_phi, z: z)
    print(f"Dataset size: {dataset_size}")

    return (dataset_rho_v_phi, dataset_rho, dataset_rho_v, dataset_rho_phi, dataset_rho_vx_vy_phi, dataset_rho_vx_vy,
            x_dataset, y_rho_dataset, y_v_dataset, y_phi_dataset, analysis_params)
