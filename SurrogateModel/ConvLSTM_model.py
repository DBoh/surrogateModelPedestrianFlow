import tensorflow as tf

import tensorflow as tf

class ConvLSTM_model(tf.keras.Model):
    def __init__(self, name,  geo_size, pred_steps, feats, kernel_size, pool_size, batch_norm, padding,  return_sequence,  **kwargs):
        self.geo_size = geo_size
        self.pred_steps = pred_steps
        self.feats = feats
        self.kernel_size = kernel_size
        self.pool_size = pool_size
        self.batch_norm = batch_norm
        self.padding = padding
        self.return_sequence = return_sequence

        super().__init__(name=name, **kwargs)

        # Inputs
        rho_in = tf.keras.Input(shape=(None, *self.geo_size, 1), name="rho")
        v_in = tf.keras.Input(shape=(None, *self.geo_size, 2), name="v")
        geo_in = tf.keras.Input(shape=(None, *self.geo_size, 2), name="geometry")

        inputs = {"rho": rho_in, "v": v_in, "geometry": geo_in}

        x = tf.keras.layers.Concatenate()([inputs["rho"], inputs["v"], inputs["geometry"]])
        # x shape: (batch, time, H, W, 5)

        # ---- ConvLSTM-Stack (optimiert) ----
        # 1) Weniger Layer, aber gleiche Filterprogression 16-32-64
        x = tf.keras.layers.ConvLSTM2D(
            filters=16,
            kernel_size=kernel_size,
            padding="same",
            return_sequences=True,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_16_1",
        )(x)

        x = tf.keras.layers.ConvLSTM2D(
            filters=32,
            kernel_size=kernel_size,
            padding="same",
            return_sequences=True,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_32_1",
        )(x)

        # Letzte Schicht ohne return_sequences → nur letzter Zeitschritt
        x = tf.keras.layers.ConvLSTM2D(
            filters=64,
            kernel_size=kernel_size,
            padding="same",
            return_sequences=False,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_64_final",
        )(x)

        # ---- Final projection ----
        out_rho = tf.keras.layers.Conv2D(
            1, (3, 3), padding="same", activation="sigmoid", name="out_rho"
        )(x)
        out_v = tf.keras.layers.Conv2D(
            2, (3, 3), padding="same", activation="tanh", name="out_v"
        )(x)

        outputs = [out_rho, out_v]

        super().__init__(inputs=inputs, outputs=outputs, name=name)

    def call(self, inputs):
        return super().call(inputs)


# class ConvLSTM_model(tf.keras.Model):
#   def __init__(self, name,  geo_size, pred_steps, feats, kernel_size, pool_size, batch_norm, padding,  return_sequence,  **kwargs):
#
#       self.geo_size = geo_size
#       self.pred_steps = pred_steps
#       self.feats = feats
#       self.kernel_size = kernel_size
#       self.pool_size = pool_size
#       self.batch_norm = batch_norm
#       self.padding = padding
#       self.return_sequence = return_sequence
#
#
#       rho_in = tf.keras.Input(shape=(None,  *self.geo_size, 1), name="rho")
#       v_in = tf.keras.Input(shape=(None,  *self.geo_size, 2), name="v")
#       geo_in = tf.keras.Input(shape=(None,  *self.geo_size, 2), name="geometry")
#
#       inputs = {"rho": rho_in, "v": v_in, "geometry": geo_in}
#
#       x = tf.keras.layers.Concatenate()([inputs["rho"], inputs["v"], inputs["geometry"]])
#
#       x = tf.keras.layers.ConvLSTM2D(
#                           filters=16,
#                           kernel_size=kernel_size,
#                           padding="same",
#                           return_sequences=True,
#                           activation="tanh",
#                           recurrent_activation="sigmoid"
#                       )(x)
#       x = tf.keras.layers.ConvLSTM2D(
#           filters=16,
#           kernel_size=kernel_size,
#           padding="same",
#           return_sequences=True,
#           activation="tanh",
#           recurrent_activation="sigmoid"
#       )(x)
#       x = tf.keras.layers.ConvLSTM2D(
#           filters=32,
#           kernel_size=kernel_size,
#           padding="same",
#           return_sequences=True,
#           activation="tanh",
#           recurrent_activation="sigmoid"
#       )(x)
#       x = tf.keras.layers.ConvLSTM2D(
#           filters=32,
#           kernel_size=kernel_size,
#           padding="same",
#           return_sequences=True,
#           activation="tanh",
#           recurrent_activation="sigmoid"
#       )(x)
#       x = tf.keras.layers.ConvLSTM2D(
#           filters=64,
#           kernel_size=kernel_size,
#           padding="same",
#           return_sequences=True,
#           activation="tanh",
#           recurrent_activation="sigmoid"
#       )(x)
#       x = tf.keras.layers.ConvLSTM2D(
#           filters=64,
#           kernel_size=kernel_size,
#           padding="same",
#           return_sequences=False,
#           activation="tanh",
#           recurrent_activation="sigmoid"
#       )(x)
#
#               # ---- Final projection ----
#       out_rho = tf.keras.layers.Conv2D(1, (3, 3), padding="same", activation="sigmoid", name="out_rho")(x)
#       out_v = tf.keras.layers.Conv2D(2, (3, 3), padding="same", activation="tanh", name="out_v")(x)
#
#       outputs = [out_rho, out_v]
#
#
#       super().__init__(inputs=inputs, outputs=outputs, name=name)

#  def call(self, inputs):
#    return super().call(inputs)
