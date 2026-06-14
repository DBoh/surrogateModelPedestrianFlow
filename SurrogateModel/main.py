import os
import sys
from pathlib import Path
from datetime import datetime
import json

import tensorflow as tf
#import keras_tuner as kt

from ConvLSTM_UNet_config import ConvLSTM_UNet_config
import ConvLSTM_UNet_model
from ConvLSTM_UNet_model import ConvLSTM_UNet_model
from ConvLSTM_config import ConvLSTM_config
import ConvLSTM_model
from Dataset import Dataset


def get_project_root():
    current_file = Path(__file__).resolve()
    return current_file.parent.parent

## Hauptroutine für den Start von Training und Vorhersage
def run_cnn(paths, time_stride, feature_time_steps, label_time_steps, pred_steps, initial_step_pred,
            learning_rate, beta_1, beta_2, batch_norm,
            feats,
            kernel_size, pool_size, padding,  return_sequence, block_numb, input, output,
            epoch, experiment_name=None, loss_weights=[1, 1]):

    tpu = False
    use_gpu = False
    
    # tpu check
    try:
        tpu = tf.distribute.cluster_resolver.TPUClusterResolver()  # TPU detection
        tf.config.experimental_connect_to_cluster(tpu)
        tf.tpu.experimental.initialize_tpu_system(tpu)
        
        print("TPU gefunden: ", tf.config.list_logical_devices('TPU'))
        tpu_strategy = tf.distribute.TPUStrategy(tpu)
        print(f"TPU wird für Training verwendet. Anzahl der Repliken: {tpu_strategy.num_replicas_in_sync}")
    except ValueError:
        print('Keine TPU gefunden. Versuche GPU...')
        
        # gpu check
        gpus = tf.config.list_physical_devices('GPU')
        if gpus:
            try:
                for gpu in gpus:
                    tf.config.experimental.set_memory_growth(gpu, True)
                
                strategy = tf.distribute.MirroredStrategy()
                print(f"GPU(s) gefunden: {len(gpus)}")
                print(f"Geräte: {strategy.extended.worker_devices}")
                use_gpu = True
            except RuntimeError as e:
                print(f"GPU-Konfigurationsfehler: {e}")
        else:
            print("Keine GPU gefunden. CPU wird verwendet.")

    device_name = tf.test.gpu_device_name()
    if use_gpu:
        print(f"Standard-GPU-Gerät: {device_name}")
        
        with tf.device('/device:GPU:0'):
            random_image_gpu = tf.random.normal((100, 100, 100, 3))
            net_gpu = tf.keras.layers.Conv2D(32, 7)(random_image_gpu)
            tf.math.reduce_sum(net_gpu)
            print(f"gpu test auf: {device_name}")

    # neue struktur (listen) oder alte (verzeichnisse)
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
        batch_size = 16 # für CPU: 8 oder 16 wählen

    ### LOAD DATASET
    dataset = Dataset(files=files, feature_time_steps=feature_time_steps, pred_steps=pred_steps,
                      label_time_steps=label_time_steps, size_geometry=size_geometry, batch_size=batch_size,
                      time_stride=time_stride, )

    dataset.load_data()

    #####

    # dataset.show_feature() # Visualisierung einzelner Daten

    if experiment_name is None:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M")
        name = f'convlstm_unet_'
        name = name + 'rho_v_geo_1_2_Decoder'
        experiment_name = f"{name}_{timestamp}"
    else:
        name = '_'.join(experiment_name.split('_')[:-2])

    ### Define Directories for saving
    directory = current_dir
    experiment_folder = f"{experiment_name}_{size_geometry[0]}"
    # save_path ist datei-präfix, nicht ordner
    save_path = os.path.join(current_dir, "model_runs", experiment_folder, experiment_folder)

    load_path = os.path.join("C:/Users/doris/PycharmProjects/ping/SurrogateModel/model_runs/Variante_D_h1_norm/output/lr00001_loss1-1_feats16_20260601_1652_64.keras")

    load = False
    base = False
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
                                   batch_norm=batch_norm, feats=feats, kernel_size=kernel_size,
                                   pool_size=pool_size, padding=padding,
                                   return_sequence=return_sequence,
                                   block_numb=block_numb,
                                   input=input, output=output, loss_weights=loss_weights)
                net.build_model_(load=load)  # compile and train model

                net.predict()
        else:
            net = ConvLSTM_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                  dataset_val=dataset.dataset_val,
                                  dataset_test=dataset.dataset_test,
                                  name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                  save_path=save_path, load_path=load_path,
                                  directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                  geo_size=size_geometry,
                                  learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                  batch_norm=batch_norm, feats=feats, kernel_size=kernel_size,
                                  pool_size=pool_size, padding=padding,
                                  return_sequence=return_sequence,
                                  block_numb=block_numb,
                                  input=input, output=output, loss_weights=loss_weights)
            net.build_model_(load=load)  # compile and train model

            net.predict()

    # dataset = net.dataset
    else:
        if tpu:
            with tpu_strategy.scope():
                net = ConvLSTM_UNet_config(dataset, dataset_train=dataset.dataset_train, dataset_val=dataset.dataset_val,
                                           dataset_test=dataset.dataset_test,
                                           name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                           save_path=save_path, load_path=load_path,
                                           directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                           geo_size=size_geometry,
                                           learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                           batch_norm=batch_norm, feats=feats,
                                           kernel_size=kernel_size,
                                           pool_size=pool_size, padding=padding,
                                           return_sequence=return_sequence,
                                           block_numb=block_numb, input=input, output=output, loss_weights=loss_weights)
                net.build_model_(load=load)
        elif use_gpu:
            with strategy.scope():
                net = ConvLSTM_UNet_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                           dataset_val=dataset.dataset_val,
                                           dataset_test=dataset.dataset_test,
                                           name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                           save_path=save_path, load_path=load_path,
                                           directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                           geo_size=size_geometry,
                                           learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                           batch_norm=batch_norm, feats=feats, kernel_size=kernel_size,
                                           pool_size=pool_size, padding=padding,
                                           return_sequence=return_sequence,
                                           block_numb=block_numb,
                                           input=input, output=output, loss_weights=loss_weights)
                net.build_model_(load=load)
        else:
            net = ConvLSTM_UNet_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                       dataset_val=dataset.dataset_val,
                                       dataset_test=dataset.dataset_test,
                                       name=name, batch_size=batch_size, epoch=epoch, time_stride=time_stride,
                                       save_path=save_path, load_path=load_path,
                                       directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                       geo_size=size_geometry,
                                       learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                       batch_norm=batch_norm, feats=feats, kernel_size=kernel_size,
                                       pool_size=pool_size, padding=padding,
                                        return_sequence=return_sequence,
                                       block_numb=block_numb,
                                       input=input, output=output, loss_weights=loss_weights)
            net.build_model_(load=load)  # compile and train model

        net.predict()
        # dataset = net.dataset

    return dataset, net


def run_tuning(dataset, tuning, learning_rate, beta_1, beta_2,
               batch_norm,
               feats, kernel_size, pool_size, padding, return_sequence, block_numb, input,
               output,
               epoch):
    ### DEFINE PARAMETERS FOR MODEL
    
    if not tuning:
        return
    
    # tuning code (aktuell aus)

    name = f'convlstm_unet_'

    name = name + 'rho_v_geo_1_2_Decoder_tuning'

    ### Define Directories for saving

    directory = current_dir
    save_path = os.path.join(current_dir, name + '_' + str(size_geometry[0]), name + '_' + str(size_geometry[0]))
    load_path = os.path.join(current_dir, name + '_' + str(size_geometry[0]),
                             'convlstm_unet_rho_v_geo_1_2Decoder_64.keras')

    load = False

    if not tuning:
        pass
    else:
        net = ConvLSTM_UNet_config(dataset=dataset, dataset_train=dataset.dataset_train,
                                   dataset_val=dataset.dataset_val,
                                   dataset_test=dataset.dataset_test,
                                   name=name, batch_size=dataset.batch_size, epoch=epoch, time_stride=time_stride,
                                   save_path=save_path, load_path=load_path,
                                   directory=directory, initial_step_pred=initial_step_pred, pred_steps=pred_steps,
                                   geo_size=size_geometry,
                                   learning_rate=learning_rate, beta_1=beta_1, beta_2=beta_2,
                                   batch_norm=batch_norm, feats=feats, kernel_size=kernel_size,
                                   pool_size=pool_size, padding=padding,
                                   return_sequence=return_sequence,
                                   block_numb=block_numb,
                                   input=input, output=output, loss_weights=loss_weights)
        tuner = kt.Hyperband(model_builder,
                             objective='val_loss',
                             max_epochs=100,
                             factor=3,
                             directory='my_dir',
                             project_name='intro_to_kt')

        early_stopping = tf.keras.callbacks.EarlyStopping(monitor="val_loss", patience=5, restore_best_weights=True,
                                                          start_from_epoch=10)
        reduce_lr = tf.keras.callbacks.ReduceLROnPlateau(monitor="val_loss", factor=0.1, patience=2)

        tuner.search(dataset.dataset_train, epochs=net.epoch,
                     batch_size=net.batch_size,
                     validation_data=dataset.dataset_val,
                     callbacks=[early_stopping, reduce_lr])

        best_hps = tuner.get_best_hyperparameters(num_trials=1)[0]
        print(best_hps)

        model = tuner.hypermodel.build(best_hps)
        history = model.fit(dataset.dataset_train, epochs=50,
                            batch_size=net.batch_size,
                            validation_data=dataset.dataset_val,
                            callbacks=[early_stopping, reduce_lr]
                            )
        val_acc_per_epoch = history.history['val_loss']
        best_epoch = val_acc_per_epoch.index(max(val_acc_per_epoch)) + 1
        print('Best epoch: %d' % (best_epoch,))

        hypermodel = tuner.hypermodel.build(best_hps)

        hypermodel.fit(dataset.dataset_train, epochs=best_epoch)


def model_builder(hp):
    hp_learning_rate = hp.Choice('learning_rate', values=[1e-2, 1e-3, 1e-4])
    hp_block = hp.Choice('block_numb', values=[2, 3])

    model = ConvLSTM_UNet_model('model', 50, 5, (3, 3), (1, 2, 2),
                                False,
                                'same', 'tanh', 'tanh', 0.00000001, 0.0,
                                0.0, True, -1)
    input_dict = {'rho': tf.keras.layers.Input(shape=(None, 64, 64, 1), name='rho'),
                  'v': tf.keras.layers.Input(shape=(None, 64, 64, 2), name='v'),
                  'geometry': tf.keras.layers.Input(shape=(None, 64, 64, 2), name='geometry')}

    model = tf.keras.Model(inputs=input_dict,
                           outputs=model.call(input_dict, block_numb=hp_block, input=['rho', 'v', 'geometry'],
                                              output=['rho', 'v']))

    model.compile(tf.keras.optimizers.Adam(learning_rate=hp_learning_rate),
                  loss={'out_rho': 'mse', 'out_v': 'cosine_similarity'}, jit_compile=False,
                  metrics=['mse', 'cosine_similarity'],
                  run_eagerly=True)
    model.summary(expand_nested=True)

    return model

####################################################################
if __name__ == '__main__':

    project_root = get_project_root()
    current_dir = os.path.join(project_root, 'SurrogateModel')
    print(project_root)
    #try:
    from get_latest_run_paths import get_tfrecord_paths_for_run
    #except ImportError:
     #   print("fehler: get_latest_run_paths.py nicht gefunden")
      #  print("Stelle sicher, dass die Datei im Projekt-Root existiert.")
       # sys.exit(1)

    run_name = None  # None = neuester run
    
    try:
        tfrecord_dir, split_paths = get_tfrecord_paths_for_run(run_name)
        actual_run_name = Path(tfrecord_dir).parent.name
        print(f"Verwende Daten aus: {tfrecord_dir}")
        print(f"Run: {actual_run_name}")
        print(f"Train: {len(split_paths['train'])} Dateien")
        print(f"Validation: {len(split_paths['validation'])} Dateien")
        print(f""
              f"Test: {len(split_paths['test'])} Dateien")
    except ValueError as e:
        print(f"FEHLER: {e}")
        if run_name:
            print(f"Run '{run_name}' nicht gefunden. Verfügbare Runs:")
            runs_dir = Path(project_root) / "data" / "runs"
            if runs_dir.exists():
                for d in sorted(runs_dir.iterdir()):
                    if d.is_dir():
                        print(f"  - {d.name}")
        else:
            print("Bitte erst 'python generate/DataGeneration.py' ausführen")
        sys.exit(1)

    size_geometry = (64, 64)

    paths = [split_paths['test'],
             split_paths['train'],
             split_paths['validation']
             ]
    print(paths)
    ### CONFIGURATION FOR DATASET

    time_stride = 1
    feature_time_steps = time_stride * 5  # length of frames considered for training
    label_time_steps = time_stride * 1  # length for comparison: input feature_time_steps, predicts next time step, this one is compared to label_time_step
    pred_steps = 50 # number of time steps that will be predicted, should be smaller than total number of time steps in trainings data
    initial_step_pred = 1

    ### DEFINE PARAMETERS FOR MODEL

    learning_rate = 1e-4
    loss_weights = [1, 1]
    
    beta_1 = 0.9  # adams paper
    beta_2 = 0.999  # adams paper
    activation = 'tanh' # wird überschrieben
    batch_norm = False
    feats = 16 # number of initial filters of first layer

    kernel_size = (3, 3)
    pool_size = (1, 2, 2)
    padding = 'same'
    return_sequence = False # if false, only final output is given # wert wird momentan überschrieben
    block_numb = 3
    input = ['rho', 'v', 'geometry']
    output = ['rho', 'v']
    epoch = 100

    timestamp = datetime.now().strftime("%Y%m%d_%H%M")

    name_parts = [
        f"lr{str(learning_rate).replace('.', '')}",
        f"loss{loss_weights[0]}-{loss_weights[1]}",
        f"feats{feats}"
    ]
    
    experiment_name = f"{'_'.join(name_parts)}_{timestamp}"
    experiment_folder = f"{experiment_name}_{size_geometry[0]}"

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
            'block_numb': block_numb,
            'kernel_size': kernel_size,
            'pool_size': pool_size,
            'padding': padding,
            'return_sequence': return_sequence,
            'input': input,
            'output': output
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
    
    # Warnung wenn Experiment-Ordner bereits existiert
    if os.path.exists(exp_dir):
        print(f"WARNUNG: Experiment-Ordner existiert bereits: {exp_dir}")
        print("         alte dateien könnten überschrieben werden")
    
    os.makedirs(exp_dir, exist_ok=True)
    
    config_path = os.path.join(exp_dir, 'config.json')
    with open(config_path, 'w') as f:
        json.dump(experiment_config, f, indent=2)
    print(f"Experiment-Config gespeichert in: {config_path}")

    ############RUN MODEL
    dataset, net = run_cnn(paths, time_stride, feature_time_steps, label_time_steps, pred_steps,
                           initial_step_pred,
                           learning_rate, beta_1, beta_2, batch_norm,
                           feats,
                           kernel_size, pool_size, padding,  return_sequence, block_numb, input, output,
                           epoch, experiment_name=experiment_name, loss_weights=loss_weights)

    #############RUN TUNING
    tuning = False

    if tuning:
       run_tuning(dataset, tuning, learning_rate, beta_1, beta_2,
               batch_norm,
               feats, kernel_size, pool_size, padding, return_sequence, block_numb, input,
               output,epoch)
    
    ############# CREATE EXPERIMENT SUMMARY
    try:
        from visualization_utils import create_experiment_summary
        summary_path = create_experiment_summary(
            exp_dir,
            history=net.history if hasattr(net, 'history') else None,
            dataset=dataset if hasattr(dataset, 'predictions') else None,
            config=experiment_config
        )
        print(f"Experiment Summary erstellt in: {summary_path}")
    except ImportError:
        print("Info: visualization_utils.py nicht gefunden - Summary wird übersprungen")
    except Exception as e:
        print(f"Warnung: Summary konnte nicht erstellt werden: {e}")
