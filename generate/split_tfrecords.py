"""
Teilt TFRecord-Dateien in Train/Validation/Test-Sets auf.
Wird vom SurrogateModel über get_latest_run_paths.py verwendet.
"""

import os
import random
import glob
import logging

logger = logging.getLogger(__name__)


def get_split_paths(source_dir, train_ratio=0.7, val_ratio=0.2, test_ratio=0.1, random_seed=42):
    """teilt tfrecord-dateien in train/val/test auf (70/20/10)"""
    # Validierung
    if not os.path.exists(source_dir):
        raise FileNotFoundError(f"Verzeichnis {source_dir} existiert nicht")
    
    if abs((train_ratio + val_ratio + test_ratio) - 1.0) > 0.001:
        raise ValueError(f"Verhältnisse müssen 1.0 ergeben, sind aber {train_ratio + val_ratio + test_ratio}")
    
    # Finde alle TFRecord-Dateien
    tfrecord_files = sorted(glob.glob(os.path.join(source_dir, '*.tfrecord')))
    
    if not tfrecord_files:
        raise ValueError(f"Keine TFRecord-Dateien in {source_dir} gefunden")
    
    logger.info(f"Gefunden: {len(tfrecord_files)} TFRecord-Dateien")
    
    # Zufällige Reihenfolge für faire Aufteilung
    random.seed(random_seed)
    random.shuffle(tfrecord_files)
    
    # Berechne Split-Größen
    total = len(tfrecord_files)
    
    # spezialfälle für kleine datenmengen
    if total == 1:
        # Bei 1 Datei: Alles für Training
        train_size, val_size, test_size = 1, 0, 0
        logger.warning("Nur 1 Datei - alles für Training verwendet")
    elif total == 2:
        # Bei 2 Dateien: Train + Val (Early Stopping spart Zeit beim Testen)
        train_size, val_size, test_size = 1, 1, 0
    elif total == 3:
        # Bei 3 Dateien: 1-1-1 (alle Phasen testen, Training eh nur Test)
        train_size, val_size, test_size = 1, 1, 1
    elif total == 4:
        # Bei 4 Dateien: 2 Train, 1 Val, 1 Test
        train_size, val_size, test_size = 2, 1, 1
    elif total == 5:
        # Bei 5 Dateien: 3 Train, 1 Val, 1 Test
        train_size, val_size, test_size = 3, 1, 1
    else:
        # Ab 6 Dateien: Normale prozentuale Aufteilung (70/20/10)
        train_size = int(total * train_ratio)
        val_size = int(total * val_ratio)
        test_size = total - train_size - val_size  # Rest geht an Test
    
    # Teile die Dateien auf
    train_files = tfrecord_files[:train_size]
    val_files = tfrecord_files[train_size:train_size + val_size]
    test_files = tfrecord_files[train_size + val_size:]
    
    logger.info(f"Aufteilung: {len(train_files)} Train, {len(val_files)} Val, {len(test_files)} Test")
    
    return {
        'train': train_files,
        'validation': val_files,
        'test': test_files
    }


if __name__ == "__main__":
    # Beispiel-Verwendung zum Testen
    logging.basicConfig(level=logging.INFO, format='%(message)s')
    
    import sys
    if len(sys.argv) > 1:
        source_dir = sys.argv[1]
    else:
        # Standard-Testpfad
        project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        source_dir = os.path.join(project_root, "data", "runs", "run_test", "tfrecords")
    
    try:
        paths = get_split_paths(source_dir)
        print(f"\nErgebnis:")
        print(f"  Train: {len(paths['train'])} Dateien")
        print(f"  Validation: {len(paths['validation'])} Dateien")
        print(f"  Test: {len(paths['test'])} Dateien")
        
        if paths['train']:
            print(f"\nBeispiel Train-Datei: {os.path.basename(paths['train'][0])}")
    except Exception as e:
        print(f"Fehler: {e}")
        sys.exit(1)