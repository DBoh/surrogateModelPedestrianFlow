import os
import sys
from pathlib import Path
from datetime import datetime
import json

import tensorflow as tf
from tensorflow import keras
# import keras_tuner as kt

from ConvLSTM_UNet_config import ConvLSTM_UNet_config
import ConvLSTM_UNet_model
from ConvLSTM_UNet_model import ConvLSTM_UNet_model
from ConvLSTM_config import ConvLSTM_config
from Dataset import Dataset

# sys.path.append("/home/dbohnet/PycharmProjects/ping")
sys.path.append(r"C:\Users\doris\PycharmProjects\ping")


def get_project_root():
    current_file = Path(__file__).resolve()
    return current_file.parent.parent


# ------------
# RUN ROUTINE
# ------------

def run_cnn(paths, time_stride, feature_time_steps, label_time_steps, pred_steps, initial_step_pred,
            learning_rate, beta_1, beta_2, batch_norm,
            feats, padding,
            epoch, load, load_path, base, experiment_name=None, loss_weights=[1, 1]):
    tpu = False
    use_gpu = False

    # ---------
    # (1) Try if TPU or GPU can be used for training
    # ---------

    try:
        tpu = tf.distribute.cluster_resolver.TPUClusterResolver()  # TPU detection
        tf.config.experimental_connect_to_cluster(tpu)
        tf.tpu.experimental.initialize_tpu_system(tpu)

        print("TPU not found: ", tf.config.list_logical_devices('TPU'))
        tpu_strategy = tf.distribute.TPUStrategy(tpu)
        print(f"TPU not used. Number of replika: {tpu_strategy.num_replicas_in_sync}")
    except ValueError:
        print('No TPU found. Try GPU...')

        # gpu check
        gpus = tf.config.list_physical_devices('GPU')
        if gpus:
            try:
                for gpu in gpus:
                    tf.config.experimental.set_memory_growth(gpu, True)

                strategy = tf.distribute.MirroredStrategy()
                print(f"GPU(s) not found: {len(gpus)}")
                print(f"Device: {strategy.extended.worker_devices}")
                use_gpu = True
            except RuntimeError as e:
                print(f"GPU configuration error: {e}")
        else:
            print("No GPU used. CPU used.")

    device_name = tf.test.gpu_device_name()
    if use_gpu:
        print(f"Standard-GPU-device: {device_name}")

        with tf.device('/device:GPU:0'):
            random_image_gpu = tf.random.normal((100, 100, 100, 3))
            net_gpu = tf.keras.layers.Conv2D(32, 7)(random_image_gpu)
            tf.math.reduce_sum(net_gpu)
            print(f"gpu test auf: {device_name}")

    #
    if isinstance(paths[0], list):
        files = paths
    else:
        files = [[], [], []]
        for idx, path in enumerate(paths):
            if os.path.isdir(path):
                for f in os.listdir(path):
                    if f.endswith('.tfrecord'):
                        full_path = os.path.join(path, f)
                        files[idx].append(full_path)
            elif path.endswith('.tfrecord'):
                files[idx].append(path)
            else:
                print(f"No File or Directory: {path}")

    if tpu:
        batch_size = 2 * tpu_strategy.num_replicas_in_sync
        print(tpu_strategy.num_replicas_in_sync)
    elif use_gpu:
        batch_size = 5 * len(gpus)
    else:
        batch_size = 16  # depend on CPU: 8 or 16 is a good choice, in general

    # ---------
    # (2) Load Dataset
    # ---------

    dataset = Dataset(files=files, feature_time_steps=feature_time_steps, pred_steps=pred_steps,
                      label_time_steps=label_time_steps, size_geometry=size_geometry, batch_size=batch_size,
                      time_stride=time_stride )

    dataset.load_data()

    if experiment_name is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M")
        name = f'convlstm_unet_'
        name = name + 'rho_v_geo_1_2_Decoder'
        experiment_name = f"{name}_{timestamp}"
    else:
        name = '_'.join(experiment_name.split('_')[:-2])

    # ---------
    # (3) Define directories for results
    # ---------

    directory = current_dir
    experiment_folder = f"{experiment_name}_{size_geometry[0]}"

    save_path = os.path.join(current_dir, "model_runs", experiment_folder, experiment_folder)

    from keras import mixed_precision
    mixed_precision.set_global_policy("float32")
    # baseline model: ConvLSTM

    # ---------
    # (4) Call model configuration
    # ---------

    if base:
        if tpu:
            with tpu_strategy.scope():
                net = ConvLSTM_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                      dataset_val=dataset.dataset_val,
                                      dataset_test=dataset.dataset_test,
                                      name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                      save_path=save_path, load_path=load_path,
                                      directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                      geo_size=size_geometry,
                                      learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                      batch_norm=batch_norm, feats=feats, padding=padding,
                                     loss_weights=loss_weights)

        else:
            net = ConvLSTM_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                  dataset_val=dataset.dataset_val,
                                  dataset_test=dataset.dataset_test,
                                  name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                  save_path=save_path, load_path=load_path,
                                  directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                  geo_size=size_geometry,
                                  learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                  batch_norm=batch_norm, feats=feats,  padding=padding,
                                  loss_weights=loss_weights)

    # not baseline: train ConvLSTM-UNet
    else:
        if tpu:
            with tpu_strategy.scope():
                net = ConvLSTM_UNet_config(dataset, dataset_train=dataset.dataset_train,
                                           dataset_val=dataset.dataset_val,
                                           dataset_test=dataset.dataset_test,
                                           name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                           save_path=save_path, load_path=load_path,
                                           directory=directory, initial_step_pred=initial_step_pred,
                                           pred_steps=pred_steps,
                                           geo_size=size_geometry,
                                           learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                           batch_norm=batch_norm, feats=feats,
                                           padding=padding,
                                          loss_weights=loss_weights)

        elif use_gpu:
            with strategy.scope():
                net = ConvLSTM_UNet_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                           dataset_val=dataset.dataset_val,
                                           dataset_test=dataset.dataset_test,
                                           name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                           save_path=save_path, load_path=load_path,
                                           directory=directory, initial_step_pred=initial_step_pred,
                                           pred_steps=pred_steps,
                                           geo_size=size_geometry,
                                           learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                           batch_norm=batch_norm, feats=feats,
                                            padding=padding,
                                           loss_weights=loss_weights)
        else:
            net = ConvLSTM_UNet_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                       dataset_val=dataset.dataset_val,
                                       dataset_test=dataset.dataset_test,
                                       name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                       save_path=save_path, load_path=load_path,
                                       directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                       geo_size=size_geometry,
                                       learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                       batch_norm=batch_norm, feats=feats, padding=padding,
                                       loss_weights=loss_weights)

        # ---------
        # (5) Build & save model
        # ---------
        net.build_model_(load=load)  # compile and train model

        # Important: get rid of optimizer before saving
        net.model.optimizer = None
        save_file = Path(save_path) / "model_registered.keras"
        net.model.save(save_file)
        # ---------
        # (6) Predict
        # ---------
        net.predict()

    return dataset, net


###################################################################
# MAIN ROUTINE: DEFINE ALL PARAMETERS
####################################################################

if __name__ == '__main__':

    # ---------
    # (1) Setting of Paths
    # ---------
    project_root = get_project_root()
    current_dir = os.path.join(project_root, 'SurrogateModel')

    try:
        from get_latest_run_paths import get_tfrecord_paths_for_run
    except ImportError:
        print("Error: get_latest_run_paths.py not found")
        print("File should be in project root folder.")
        sys.exit(1)

    run_name = None  # None = newest run

    try:
        tfrecord_dir, split_paths = get_tfrecord_paths_for_run(run_name)
        actual_run_name = Path(tfrecord_dir).parent.name
        print(f"Taking data from: {tfrecord_dir}")
        print(f"Run: {actual_run_name}")
        print(f"Train: {len(split_paths['train'])} Files")
        print(f"Validation: {len(split_paths['validation'])} Files")
        print(f""
              f"Test: {len(split_paths['test'])} Files")
    except ValueError as e:
        print(f"FEHLER: {e}")
        if run_name:
            print(f"Run '{run_name}' not found. Available training data:")
            runs_dir = Path(project_root) / "data" / "runs"
            if runs_dir.exists():
                for d in sorted(runs_dir.iterdir()):
                    if d.is_dir():
                        print(f"  - {d.name}")
        else:
            print("Run first 'python generate/DataGeneration.py' or check your project folder structure.")
        sys.exit(1)

    size_geometry = (64, 64)  # fixed geometry size

    paths = [split_paths['test'],
             split_paths['train'],
             split_paths['validation']
             ]
    print(paths)

    # ----------------
    # (2) Set TIME STEPS
    # -------------------

    time_stride = 1
    feature_time_steps = time_stride * 5  # length of frames considered for training
    label_time_steps = time_stride * 1  # length for comparison: input feature_time_steps, predicts next time step, this one is compared to label_time_step
    pred_steps = 50  # number of time steps that will be predicted, should be smaller than total number of time steps in trainings data
    initial_step_pred = 1

    # --------------------
    # (3) Set MODEL PARAMETER
    # ----------------

    learning_rate = 1e-4
    loss_weights = [1, 1]

    beta_1 = 0.9  # adams paper
    beta_2 = 0.999  # adams paper
    batch_norm = False  # should be false
    feats = 16  # number of initial filters of first layer
    padding = 'same'
    epoch = 5

    load = True  # load previously trained model
    load_path = os.path.join(
        r"C:\Users\doris\PycharmProjects\ping\SurrogateModel\model_runs\Variante_D_100_Epochen_1000weights\output\lr00001_loss1-1_feats16_20260614_1826_64.keras")
    baseline = False  # True: baseline model ConvLSTM; False: model ConvLSTM-UNet

    # ---------------
    # (4) Define folder for results
    # ---------------

    timestamp = datetime.now().strftime("%Y%m%d_%H%M")

    name_parts = [
        f"lr{str(learning_rate).replace('.', '')}",
        f"loss{loss_weights[0]}-{loss_weights[1]}",
        f"feats{feats}"
    ]

    experiment_name = f"{'_'.join(name_parts)}_{timestamp}"
    experiment_folder = f"{experiment_name}_{size_geometry[0]}"

    # -------------
    # (5) Save configuration of training
    # ----------------

    experiment_config = {
        'experiment_id': experiment_name,
        'timestamp': timestamp,
        'data_source': {
            'run_name_requested': run_name,
            'run_name_actual': actual_run_name,
            'tfrecord_dir': tfrecord_dir,
            'train_files': len(paths[1]) if len(paths) > 1 else 0,
            'val_files': len(paths[2]) if len(paths) > 2 else 0,
            'test_files': len(paths[0]) if len(paths) > 0 else 0
        },
        'hyperparameters': {
            'learning_rate': learning_rate,
            'batch_size': 5,
            'epochs': epoch,
            'feats': feats,
            'beta_1': beta_1,
            'beta_2': beta_2,
            'batch_norm': batch_norm,
            'loss_weights': loss_weights
        },
        'model_architecture': {
            'input': input
        },
        'dataset_config': {
            'time_stride': time_stride,
            'feature_time_steps': feature_time_steps,
            'label_time_steps': label_time_steps,
            'pred_steps': pred_steps,
            'initial_step_pred': initial_step_pred,
            'size_geometry': size_geometry
        }
    }
    exp_dir = os.path.join(current_dir, "model_runs", experiment_folder)

    # Warning if result folder already exists
    if os.path.exists(exp_dir):
        print(f"WARNING: Experiment-Ordner existiert bereits: {exp_dir}")
        print("         alte dateien könnten überschrieben werden")

    os.makedirs(exp_dir, exist_ok=True)

    config_path = os.path.join(exp_dir, 'config.json')
    with open(config_path, 'w') as f:
        json.dump(experiment_config, f, indent=2)
    print(f"Experiment-Config gespeichert in: {config_path}")

    # -------------
    # (6) Run model
    # ----------------
    dataset, net = run_cnn(paths, time_stride, feature_time_steps, label_time_steps, pred_steps,
                           initial_step_pred,
                           learning_rate, beta_1, beta_2, batch_norm,
                           feats,
                           padding,
                           epoch, load, load_path, baseline, experiment_name=experiment_name, loss_weights=loss_weights)

    # -------------
    # (7) Save results
    # ----------------
    try:
        from visualization_utils import create_experiment_summary

        summary_path = create_experiment_summary(
            exp_dir,
            history=net.history if hasattr(net, 'history') else None,
            dataset=dataset if hasattr(dataset, 'predictions') else None,
            config=experiment_config
        )
        print(f"Experiment Summary in: {summary_path}")
    except ImportError:
        print("Info: visualization_utils.py not found - Summary not written")
    except Exception as e:
        print(f"Warning: Summary can't be written: {e}")
