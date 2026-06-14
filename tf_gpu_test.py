#!/usr/bin/env python3
# gpu test

import tensorflow as tf
import sys

print("tensorflow gpu test")
print(f"tf version: {tf.__version__}")

# gpu suchen
gpus = tf.config.list_physical_devices('GPU')

if not gpus:
    print("keine gpu gefunden")
    print("check nvidia treiber, cuda, cudnn")
    sys.exit(1)

print(f"gpus: {len(gpus)}")

# test
try:
    gpu = gpus[0]
    tf.config.experimental.set_memory_growth(gpu, True)
    
    # matrix mult test
    with tf.device('/device:GPU:0'):
        a = tf.constant([[1.0, 2.0], [3.0, 4.0]])
        b = tf.constant([[5.0, 6.0], [7.0, 8.0]])
        c = tf.matmul(a, b)
    
    print("test ok:")
    print(c.numpy())
    
except Exception as e:
    print(f"fehler: {e}")
    sys.exit(1)