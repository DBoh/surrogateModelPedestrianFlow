import tensorflow as tf
import numpy as np
import os
import matplotlib.pyplot as plt
from pathlib import Path

import sys
sys.path.append(r"C:\Users\doris\PycharmProjects\ping\SurrogateModel")
#sys.path.append(r"/home/dbohnet/PycharmProjects/ping/SurrogateModel")
from ConvLSTM_UNet_model import ConvLSTM_UNet_model
from ConvLSTM_UNet_config import ConvLSTM_UNet_config
sys.path.append(r"C:\Users\doris\PycharmProjects\ping")

##################
# SCRIPT to manually PREDICT frames based on single input and load model
##################


# ---------------------------------------------------------
# 1) Absoluter Pfad zu deinem Modell
# ---------------------------------------------------------
MODEL_PATH = r"C:\Users\doris\PycharmProjects\ping\SurrogateModel\model_registered.keras"
#MODEL_PATH = "/home/dbohnet/PycharmProjects/ping/SurrogateModel/model_registered.keras"

# ---------------------------------------------------------
# 2) Absoluter Pfad zu einer einzelnen TFRecord-Datei
# ---------------------------------------------------------
TFRECORD_PATH = r"C:\Users\doris\PycharmProjects\ping\SurrogateModel\data\test\test_measurements_01.tfrecord"
#TFRECORD_PATH = "/home/dbohnet/PycharmProjects/ping/SurrogateModel/data/test/test_measurements_01.tfrecord"
# ---------------------------------------------------------
# 3) Feature-Definition (muss zu deinen TFRecords passen!)
# ---------------------------------------------------------
feature_description = {
    "rho": tf.io.FixedLenFeature([], tf.string),
    "v": tf.io.FixedLenFeature([], tf.string),
    "geometry": tf.io.FixedLenFeature([], tf.string),
}

def parse_example(example_proto):
    parsed = tf.io.parse_single_example(example_proto, feature_description)



    # Rohdaten dekodieren
    rho = tf.io.parse_tensor(parsed["rho"], out_type=tf.float32)
    rho = tf.cast(rho,tf.float32)
    T_rho = tf.size(rho)//(64*64*1)

    v = tf.io.parse_tensor(parsed["v"], out_type=tf.float32)
    v = tf.cast(v,tf.float32)
    geometry = tf.io.parse_tensor(parsed["geometry"], out_type=tf.int32)
    geometry = tf.cast(geometry,tf.float32)
    T_v = tf.size(v) // (64 * 64 * 2)
    T_geo = tf.size(geometry) // (64 * 64 * 2)

    # Shape wiederherstellen
    rho = tf.reshape(rho, (T_rho,64,64, 1))
    v = tf.reshape(v, (T_v,64,64, 2))
    geometry = tf.reshape(geometry, (T_geo,64,64, 2))

    return rho, v, geometry

# ---------------------------------------------------------
# 4) TFRecord laden
# ---------------------------------------------------------
raw_dataset = tf.data.TFRecordDataset([TFRECORD_PATH])
parsed_dataset = raw_dataset.map(parse_example)

# Nur ein einziges Beispiel extrahieren
rho, v, geometry = next(iter(parsed_dataset))

# Batch-Dimension hinzufügen
rho = tf.expand_dims(rho, axis=0)

v = tf.expand_dims(v, axis=0)
geometry = tf.expand_dims(geometry, axis=0)
geometry = tf.repeat(geometry, repeats=50, axis=1)
rho = rho[:, :50]
v = v[:, :50]


# ---------------------------------------------------------
# 5) Modell laden
# ---------------------------------------------------------
from keras import mixed_precision

mixed_precision.set_global_policy("float32")
model = tf.keras.models.load_model(
    MODEL_PATH,
    custom_objects={"ConvLSTM_UNet_model": ConvLSTM_UNet_model},
    compile=False
)
# ---------------------------------------------------------
# 6) Vorhersage ausführen
# ---------------------------------------------------------

time_stride = 1
feature_time_steps = time_stride * 5  # length of frames considered for training
label_time_steps = time_stride * 1  # length for comparison: input feature_time_steps, predicts next time step, this one is compared to label_time_step
pred_steps = 50  # number of time steps that will be predicted, should be smaller than total number of time steps in trainings data
initial_step_pred = 1



# labels haben Form (batch, H, W, C)
rho_label = rho  # (batch, 1, H, W, 1)
v_label = v  # (batch, 1, H, W, 2)

# in Plot-Format bringen
rho_label = tf.transpose(rho_label, [0, 1, 4, 2, 3])  # (B,1,C,H,W)
v_label = tf.transpose(v_label, [0, 1, 4, 2, 3])

labels = (rho_label, v_label)




data = {
    'rho': rho[:, :5, :, :],
    'v': v[:, :5, :, :],
    'geometry': geometry[:, :5, :, :]
}  # wird für die iterative Vorhersage genutzt, übernimmt immer nur die letzten 5 Zeitschritte
data_temp = data  # speichert alle Zeitschritte


for step in range(45):
    predictions = model.predict(data)

    new_rho = predictions[0][::, tf.newaxis, ::, ::, ::]  # zusätzliche Zeitdim. für Outputdaten erzeugen
    rho_temp = tf.concat((data_temp['rho'], new_rho), axis=1)
    new_rho = tf.concat((data['rho'], new_rho), axis=1)  # an Inputdaten anhängen

    new_v = predictions[1][::, tf.newaxis, ::, ::, ::]
    v_temp = tf.concat((data_temp['v'], new_v), axis=1)
    new_v = tf.concat((data['v'], new_v), axis=1)

    data = {
        'rho': new_rho[:, -5:, :, :],  # letzten feature-time_steps-Zeitschritte verwenden
        'v': new_v[:, -5:, :, :],
        'geometry': geometry[:, -5:, :, :]
    }

    data_temp = {'rho': rho_temp,
                 'v': v_temp,
                 'geometry': geometry}



predictions = (np.transpose(data_temp['rho'], [0, 1, 4, 2, 3]),
               np.transpose(data_temp['v'], [0, 1, 4, 2, 3]))

print(predictions)



# ---------------------------------------------------------
# 7) Ergebnisse speichern
# ---------------------------------------------------------
output_path = os.path.join(os.path.dirname(TFRECORD_PATH), "prediction_output.npz")

np.savez(output_path,
         pred_rho=predictions[0],
         pred_v=predictions[1])

data = np.load(output_path)

print("Keys:", data.files)

# Beispiel: Arrays auslesen
pred_rho = predictions[0]   # (B, T, 1, H, W)

B, T, C, H, W = pred_rho.shape
for t in range(T):
    img = pred_rho[0,t,0]
    img2 = rho_label[0,t,0]
    fig, axs = plt.subplots(1, 2, figsize=(8, 4))

    axs[0].imshow(img2, origin="lower")
    axs[0].set_title(f"Label – t={t}")

    axs[1].imshow(img, origin="lower")
    axs[1].set_title(f"Prediction – t={t}")

    plt.tight_layout()
    img_path = os.path.join(r"C:\Users\doris\PycharmProjects\ping\SurrogateModel\model_runs", f"{t}.png")
    plt.savefig(img_path, dpi=150, bbox_inches='tight')
    #plt.show()


print(f"Vorhersage gespeichert unter: {output_path}")
