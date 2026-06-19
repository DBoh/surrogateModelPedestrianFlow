import tensorflow as tf
from tensorflow.keras.saving import register_keras_serializable
from tensorflow import keras

@register_keras_serializable(package="Custom", name="ConvLSTM_UNet_model")
class ConvLSTM_UNet_model(tf.keras.Model):
  def __init__(self, geo_size, pred_steps, feats, padding, batch_norm, name="ConvLSTM_UNet_model", **kwargs):
      super().__init__(name=name, **kwargs)
      self.geo_size = geo_size
      self.pred_steps = pred_steps
      self.feats = feats
      self.padding = padding
      self.batch_norm = batch_norm

      # Encoder – Block 1
      self.concat = keras.layers.Concatenate()
      self.enc1_lstm1 = keras.layers.ConvLSTM2D(
          filters=self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=True
      )
      self.enc1_bn = keras.layers.BatchNormalization()
      self.enc1_lstm2 = keras.layers.ConvLSTM2D(
          filters=self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=True
      )
      self.enc1_pool = keras.layers.MaxPool3D(pool_size=(1, 2, 2))

      # Encoder – Block 2
      self.enc2_lstm1 = keras.layers.ConvLSTM2D(
          filters=2 * self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=True
      )
      self.enc2_bn = keras.layers.BatchNormalization()
      self.enc2_lstm2 = keras.layers.ConvLSTM2D(
          filters=2 * self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=True
      )
      self.enc2_pool = keras.layers.MaxPool3D(pool_size=(1, 2, 2))

      # Encoder – Block 3
      self.enc3_lstm1 = keras.layers.ConvLSTM2D(
          filters=4 * self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=True
      )
      self.enc3_bn = keras.layers.BatchNormalization()
      self.enc3_lstm2 = keras.layers.ConvLSTM2D(
          filters=4 * self.feats, kernel_size=(3, 3),
          padding=self.padding, return_sequences=False
      )
      self.enc3_pool = keras.layers.MaxPool2D(pool_size=(2, 2))

      # Decoder – Up 1
      self.dec1_up = keras.layers.UpSampling2D(size=(2, 2))
      self.dec1_conv1 = keras.layers.Conv2D(4 * self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec1_act1 = keras.layers.ELU()
      self.dec1_drop = keras.layers.SpatialDropout2D(0.1)
      self.dec1_concat = keras.layers.Concatenate()
      self.dec1_conv2 = keras.layers.Conv2D(4 * self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec1_act2 = keras.layers.ELU()
      self.dec1_bn = keras.layers.BatchNormalization()

      # Decoder – Up 2
      self.dec2_up = keras.layers.UpSampling2D(size=(2, 2))
      self.dec2_conv1 = keras.layers.Conv2D(2 * self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec2_act1 = keras.layers.ELU()
      self.dec2_concat = keras.layers.Concatenate()
      self.dec2_conv2 = keras.layers.Conv2D(2 * self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec2_act2 = keras.layers.ELU()
      self.dec2_bn = keras.layers.BatchNormalization()

      # Decoder – Up 3
      self.dec3_up = keras.layers.UpSampling2D(size=(2, 2))
      self.dec3_conv1 = keras.layers.Conv2D(self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec3_act1 = keras.layers.ELU()
      self.dec3_concat = keras.layers.Concatenate()
      self.dec3_conv2 = keras.layers.Conv2D(self.feats, kernel_size=(3, 3), padding=self.padding)
      self.dec3_act2 = keras.layers.ELU()
      self.dec3_bn = keras.layers.BatchNormalization()

      # Outputs
      self.out_rho_layer = keras.layers.Conv2D(
          1, (3, 3), padding="same", activation="sigmoid", name="out_rho"
      )
      self.out_v_layer = keras.layers.Conv2D(
          2, (3, 3), padding="same", activation="tanh", name="out_v"
      )

  def call(self, inputs, training=False):

      # Fall A: Dataset:  Dictionary
      if isinstance(inputs, dict):
          rho = inputs["rho"]
          v = inputs["v"]
          geo = inputs["geometry"]

      # Fall B: Dummy‑Build: list
      else:
          rho, v, geo = inputs

          # Encoder
      x = self.concat([rho, v, geo])

      # Block 1
      x = self.enc1_lstm1(x)
      if self.batch_norm:
          x = self.enc1_bn(x, training=training)
      x = self.enc1_lstm2(x)
      conv1 = x[:, -1, :, :, :]
      x = self.enc1_pool(x)

      # Block 2
      x = self.enc2_lstm1(x)
      if self.batch_norm:
          x = self.enc2_bn(x, training=training)
      x = self.enc2_lstm2(x)
      conv2 = x[:, -1, :, :, :]
      x = self.enc2_pool(x)

      # Block 3
      x = self.enc3_lstm1(x)
      if self.batch_norm:
          x = self.enc3_bn(x, training=training)
      x = self.enc3_lstm2(x)
      conv3 = x
      x = self.enc3_pool(x)

      # Decoder – Up 1
      x = self.dec1_up(x)
      x = self.dec1_conv1(x)
      x = self.dec1_act1(x)
      x = self.dec1_drop(x, training=training)
      x = self.dec1_concat([x, conv3])
      x = self.dec1_conv2(x)
      x = self.dec1_act2(x)
      if self.batch_norm:
          x = self.dec1_bn(x, training=training)

      # Decoder – Up 2
      x = self.dec2_up(x)
      x = self.dec2_conv1(x)
      x = self.dec2_act1(x)
      x = self.dec2_concat([x, conv2])
      x = self.dec2_conv2(x)
      x = self.dec2_act2(x)
      if self.batch_norm:
          x = self.dec2_bn(x, training=training)

      # Decoder – Up 3
      x = self.dec3_up(x)
      x = self.dec3_conv1(x)
      x = self.dec3_act1(x)
      x = self.dec3_concat([x, conv1])
      x = self.dec3_conv2(x)
      x = self.dec3_act2(x)
      if self.batch_norm:
          x = self.dec3_bn(x, training=training)

      out_rho = self.out_rho_layer(x)
      out_v = self.out_v_layer(x)
      return out_rho, out_v

  def get_config(self):
      config = super().get_config()
      config.update({
          "geo_size": self.geo_size,
          "pred_steps": self.pred_steps,
          "feats": self.feats,
          "padding": self.padding,
          "batch_norm": self.batch_norm
      })
      return config

