import tensorflow as tf
from tensorflow.keras.saving import register_keras_serializable


@register_keras_serializable(package="Custom", name="ConvLSTM_model")
class ConvLSTM_model(tf.keras.Model):
    def __init__(self, geo_size, pred_steps, feats,  batch_norm, padding,
                 name="ConvLSTM_model", **kwargs):
        super().__init__(name=name, **kwargs)
        self.geo_size = geo_size
        self.pred_steps = pred_steps
        self.feats = feats
        self.batch_norm = batch_norm
        self.padding = padding


        self.concat = tf.keras.layers.Concatenate()()
        # x shape: (batch, time, H, W, 5)

        # Same filter progression as ConvLSTM-UNet 16-32-64
        self.conv1 = tf.keras.layers.ConvLSTM2D(
            filters=16,
            kernel_size=(3, 3),
            padding="same",
            return_sequences=True,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_16_1",
        )

        self.conv2 = tf.keras.layers.ConvLSTM2D(
            filters=32,
            kernel_size=(3, 3),
            padding="same",
            return_sequences=True,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_32_1",
        )

        # Last layer without return_sequences
        self.conv3 = tf.keras.layers.ConvLSTM2D(
            filters=64,
            kernel_size=(3, 3),
            padding="same",
            return_sequences=False,
            activation="tanh",
            recurrent_activation="sigmoid",
            name="convlstm_64_final",
        )

        # ---- Final projection ----
        self.out_rho_layer = tf.keras.layers.Conv2D(
            1, (3, 3), padding="same", activation="sigmoid", name="out_rho"
        )
        self.out_v_layer = tf.keras.layers.Conv2D(
            2, (3, 3), padding="same", activation="tanh", name="out_v"
        )

    def call(self, inputs, training=False):

        # Fall A: Dataset liefert ein Dictionary
        if isinstance(inputs, dict):
            rho = inputs["rho"]
            v = inputs["v"]
            geo = inputs["geometry"]

        # Fall B: Dummy‑Build liefert eine Liste
        else:
            rho, v, geo = inputs

        # Concatenation
        x = self.concat([rho, v, geo])

        # Block 1: 16
        x = self.conv1(x)
        # Block 2: 32
        x = self.conv2(x)
        # Block 3: 64
        x = self.conv3(x)

        out_rho = self.out_rho_layer(x)
        out_v = self.out_v_layer(x)
        return out_rho, out_v

    def get_config(self):
        config = super().get_config()
        config.update({
            "geo_size": self.geo_size,
            "pred_steps": self.pred_steps,
            "feats": self.feats,
            "batch_norm": self.batch_norm,
            "padding": self.padding
        })
        return config
