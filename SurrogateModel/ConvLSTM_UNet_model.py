import tensorflow as tf
from tensorflow.keras.saving import register_keras_serializable
from tensorflow import keras

@register_keras_serializable(package="Custom", name="ConvLSTM_UNet_model")
class ConvLSTM_UNet_model(tf.keras.Model):
  def __init__(self, name,  geo_size, pred_steps, feats, kernel_size, pool_size, batch_norm, padding,  return_sequence,  **kwargs):

      self.geo_size = geo_size
      self.pred_steps = pred_steps
      self.feats = feats
      self.kernel_size = kernel_size
      self.pool_size = pool_size
      self.batch_norm = batch_norm
      self.padding = padding
      self.return_sequence = return_sequence


      rho_in = tf.keras.Input(shape=(None,  *self.geo_size, 1), name="rho")
      v_in = tf.keras.Input(shape=(None,  *self.geo_size, 2), name="v")
      geo_in = tf.keras.Input(shape=(None,  *self.geo_size, 2), name="geometry")

      inputs = {"rho": rho_in, "v": v_in, "geometry": geo_in}

      """
      Encoder
      """

      x = tf.keras.layers.Concatenate()([inputs["rho"], inputs["v"], inputs["geometry"]])
      # first layer  16 x 64 x 64
      x = tf.keras.layers.ConvLSTM2D(filters=self.feats, kernel_size=(3, 3), padding=self.padding,
                                     return_sequences=True)(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      x = tf.keras.layers.ConvLSTM2D(filters=self.feats, kernel_size=(3, 3), padding=self.padding, return_sequences=True)(x)
      conv1 = x[:, -1, :, :, :]

      x = tf.keras.layers.MaxPool3D(pool_size=(1, 2, 2))(x)

      # second layer
      # 32 x 32 x 32
      x = tf.keras.layers.ConvLSTM2D(filters=2*self.feats, kernel_size=(3, 3), padding=self.padding,
                                     return_sequences=True)(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      x = tf.keras.layers.ConvLSTM2D(filters=2*self.feats, kernel_size=(3, 3), padding=self.padding,
                                     return_sequences=True)(x)
      conv2 = x[:, -1, :, :, :]  # nur letzten Zeitschritt übergeben
      x = tf.keras.layers.MaxPool3D(pool_size=(1, 2, 2))(x)

      # third layer: 64 x 16 x 16
      x = tf.keras.layers.ConvLSTM2D(filters=4*self.feats, kernel_size=(3, 3), padding=self.padding,
                                     return_sequences=True)(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      x = tf.keras.layers.ConvLSTM2D(filters=4*self.feats, kernel_size=(3, 3), padding=self.padding,
                                     return_sequences=False)(
          x)  # False: im letzten Encoder-Layer nur letzten Output zurückgeben
      conv3 = x
      x = tf.keras.layers.MaxPool2D(pool_size=(2, 2))(x)
      # 128 x 8 x 8

      """
      Decoder
      """

      ## upsampling 1 rho: 256x4x4
      x = tf.keras.layers.UpSampling2D(size=(2, 2), data_format=None, **kwargs)(x)
      x = tf.keras.layers.Conv2D(4*self.feats, kernel_size=(3, 3), padding=self.padding
                                 )(x)
      x = tf.keras.layers.ELU()(x)
      x = tf.keras.layers.SpatialDropout2D(0.1)(x)
      x = tf.keras.layers.Concatenate()([x, conv3])
      # 256 x 8 x 8
      x = tf.keras.layers.Conv2D(filters=4*self.feats, kernel_size=(3, 3), padding=self.padding
                                 )(x)
      x = tf.keras.layers.ELU()(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      #  x = tf.keras.layers.Conv2D(filters=40, kernel_size=(3, 3), activation=self.activation, padding=self.padding
      #  )(x)
      ## upsamling 2
      x = tf.keras.layers.UpSampling2D(size=(2, 2), data_format=None, **kwargs)(x)
      x = tf.keras.layers.Conv2D(2*self.feats, kernel_size=(3, 3), padding=self.padding
                                 )(x)
      x = tf.keras.layers.ELU()(x)
      x = tf.keras.layers.Concatenate()([x, conv2])
      # 64 x 16 x 16
      x = tf.keras.layers.Conv2D(filters=2*self.feats, kernel_size=(3, 3), padding=self.padding)(x)
      x = tf.keras.layers.ELU()(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      ## upsampling 3
      x = tf.keras.layers.UpSampling2D(size=(2, 2), data_format=None, **kwargs)(x)
      x = tf.keras.layers.Conv2D(self.feats, kernel_size=(3, 3), padding=self.padding
                                 )(x)
      x = tf.keras.layers.ELU()(x)
      x = tf.keras.layers.Concatenate()([x, conv1])
      # 64 x 32 x 32
      x = tf.keras.layers.Conv2D(filters=self.feats, kernel_size=(3, 3), padding=self.padding)(x)
      x = tf.keras.layers.ELU()(x)
      if self.batch_norm:
          x = tf.keras.layers.BatchNormalization()(x)

      out_rho = tf.keras.layers.Conv2D(1, (3, 3), padding="same", activation="sigmoid", name="out_rho")(x)
      out_v = tf.keras.layers.Conv2D(2, (3, 3), padding="same", activation="tanh", name="out_v")(x)

      outputs = [out_rho, out_v]

      super().__init__(inputs = inputs, outputs = outputs, name=name)


  def call(self, inputs):
    return super().call(inputs)


 #    prediction.append(out_rho)
 #    prediction.append(out_v)
 #
 #
 #    return prediction