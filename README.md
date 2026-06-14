
## Inhaltsverzeichnis

1. [Projektübersicht](#1-projektübersicht)
2. [Quickstart](#2-quickstart)
3. [Projektstruktur](#3-projektstruktur)
4. [Installation](#4-installation)
5. [Konfiguration und Parameter](#5-konfiguration-und-parameter)
6. [eFlowAgent](#6-eflowagent)
7. [generate](#7-generate)
8. [SurrogateModel](#8-surrogatemodel)
9. [Bekannte Probleme](#9-bekannte-probleme)


## 1. Projektübersicht

PInG simuliert Personenströme in Gebäuden und trainiert ein neuronales Netz für schnelle Vorhersagen.

Drei Module:

1. **generate**: Datengenerierung
   - Erstellt zufällige Gebäudelayouts 
   - Steuert die C-Simulation für jedes Layout
   - Konvertiert Ergebnisse zu TFRecords

2. **eFlowAgent**: C-Simulation 
   - Berechnet Personenströme
   - Erzeugt u.a. Dichte- und Geschwindigkeitsfelder

3. **SurrogateModel**: Neuronales Netz
   - ConvLSTM-U-Net
   - Lernt Simulationsdynamik aus TFRecords

## 2. Quickstart

Voraussetzung: Installation abgeschlossen (siehe Abschnitt 4), Python-Umgebung aktiviert.

### 2.1. C-Simulation kompilieren

```bash
cd eFlowAgent
make clean && make
```

- Erzeugt `pFlowSIM` executable
- Nur einmalig nötig (außer bei C-Code Änderungen)

### 2.2. Daten generieren

```bash
cd generate
python DataGeneration.py

- Generiert Layouts und simuliert sie mit eFlowAgent
- Speichert TFRecords in `data/runs/run_YYYYMMDD_HHMMSS/`
- Parameter in `config.py`
- Löscht alte Octave-Daten

### 2.3. Modell trainieren

```bash
cd SurrogateModel
python main.py
```

- Findet neueste Daten automatisch
- Split 70/20/10 (Train/Val/Test)
- Trainiert ConvLSTM-U-Net
- Output in `model_runs/`

## 3. Projektstruktur

```
ping/
├── README.md
├── requirements.txt
├── get_latest_run_paths.py        # Findet neueste Daten
├── tf_gpu_test.py                 # GPU-Test
│
├── eFlowAgent/                    # C-Simulation
│   ├── makefile
│   ├── pFlowSIM                   # (auto) Nach 'make'
│   ├── 00Input/
│   │   └── eFlow.dat              # Wird überschrieben
│   ├── Octave/                    # (auto) Temporäre .dat Dateien
│   └── VideoCreator/              # MATLAB Visualisierung
│
├── generate/                      # Datengenerierung
│   ├── DataGeneration.py          # Hauptskript
│   ├── config.py                  # Zentrale Konfiguration
│   ├── create_layout.py           # Layout-Generator
│   ├── DataGeneration_json.py     # Geometrie-Verarbeitung
│   ├── DataGeneration_tfrecord.py # .dat zu TFRecord
│   ├── split_tfrecords.py         # Train/Val/Test Split
│   └── visualize_layouts.py       # Visualisierung
│
├── SurrogateModel/                # Neuronales Netz
│   ├── main.py                    # Training
│   ├── ConvLSTM_UNet_model.py     # Modell-Architektur
│   ├── ConvLSTM_UNet_config.py    # Modell-Konfiguration
│   ├── Dataset.py                 # Data Loading
│   ├── visualization_utils.py     # Visualisierung
│   └── model_runs/                # (auto) Trainierte Modelle
│
└── data/                          # (auto) Generierte Daten
    └── runs/
        └── run_YYYYMMDD_HHMMSS/   # (auto) Pro Generierung
            ├── config.json
            ├── layouts/*.json
            └── tfrecords/*.tfrecord
```

(auto) = wird automatisch erstellt, muss nicht manuell angelegt werden

## 4. Installation

### 4.1. Systemanforderungen

- C Compiler (gcc/clang)
- Python 3.10-3.12 (3.13 noch nicht kompatibel mit TensorFlow)
- Graphviz für Modell-Architektur-Diagramme
- Optional: Octave/MATLAB für .dat Visualisierung
- Optional: Nvidia-Treiber, CUDA, cuDNN für GPU-Beschleunigung

### 4.2. Installation

Bei älteren Systemen wo `python` noch auf Python 2 zeigt: `python3` verwenden.

#### 4.2.1. C Compiler

```bash
# Prüfen ob vorhanden
gcc --version    # oder
clang --version  # oder
cc --version

# Installation falls nötig:
# macOS (meist schon vorhanden)
brew install gcc

# Linux
sudo apt install build-essential

# Windows: WSL empfohlen, alternativ MinGW
```

#### 4.2.2. Python

```bash
# Version prüfen
python --version  # Sollte 3.10-3.12 sein

# Falls nötig installieren:
# macOS
brew install python@3.11

# Linux
sudo apt install python3 python3-pip python3-venv

# Windows: python.org Download
```

#### 4.2.3. Virtual Environment

```bash
# Ins Projektverzeichnis wechseln
cd /pfad/zu/ping

# Erstellen
python -m venv .venv

# Aktivieren
source .venv/bin/activate  # Linux/Mac
.venv\Scripts\activate     # Windows

# Dependencies installieren
pip install -r requirements.txt
```

- Bei jeder neuen Terminal-Session: Virtual Environment aktivieren
- IDE-Terminals brauchen i.d.R. auch manuelle Aktivierung
- IDE Run-Buttons verwenden die konfigurierte venv i.d.R. automatisch

#### 4.2.4. Graphviz

```bash
# Prüfen ob vorhanden
dot --version

# Installation falls nötig:
# macOS
brew install graphviz

# Linux
sudo apt install graphviz

# Windows: graphviz.org Download
```

### 4.3. Git-Setup

```bash
# eFlow.dat vor versehentlichem Commit schützen
git update-index --skip-worktree eFlowAgent/00Input/eFlow.dat
```

- eFlow.dat wird bei jeder Simulation überschrieben (enthält Pfad zur JSON)
- Muss als Template im Repo bleiben
- Lokale Änderungen sollen nicht committed werden

### 4.4. GPU-Setup (optional)

Für NVIDIA GPUs:
1. NVIDIA Treiber installieren
2. CUDA (Version muss zu TensorFlow passen - siehe tensorflow.org)
3. cuDNN passend zu CUDA
4. Test: `nvidia-smi` und `python tf_gpu_test.py`

Ohne GPU verwendet TensorFlow CPU (funktioniert, dauert nur länger).


## 5. Konfiguration und Parameter

### 5.1. Übersicht

**generate - Datengenerierung:**
- `config.py` - Zentrale Konfiguration (TimeConfig, DataConfig, LayoutConfig, SimulationConfig, TechnicalConstants)
- Details siehe Abschnitt 7.4

**eFlowAgent - C-Simulation:**
- Hardcoded in C: `values.c`, `octaveplot.c` (siehe 6.4)
- Dynamisch über JSON: Parameter aus `SimulationConfig` (siehe 5.3 Parameter-Flow)
- C-Python Synchronisation beachten (siehe 5.2)

**SurrogateModel - Neuronales Netz:**
- Training und Architektur: `main.py:344-378`
- Dataset: `Dataset.py`, `main.py:311,335`
- Optimizer: `ConvLSTM_UNet_config.py:30`
- Details siehe Abschnitt 8.4

### 5.2. Parameter-Synchronisation zwischen C und Python

Folgende Parameter sind in C hardcoded und müssen manuell mit `config.py` synchron gehalten werden:

**Zeit-Parameter:**
- C: `PED.delta = 0.1` (values.c:80) -> Python: `C_DELTA = 0.1` (config.py:49)
- C: `PED.plotfreq = 10` (values.c:103) -> Python: `C_PLOTFREQ = 10` (config.py:50)

**Räumliche Auflösung:**
- C: `alphax=1./64.` (octaveplot.c:16) -> Python: `GEOMETRY_SIZE = (64, 64)` (config.py:6)

Automatisch berechnet:
- `TIME_STEPS_PER_SECOND` aus `C_DELTA`/`C_PLOTFREQ`
- `TFRECORD_MAX_STEPS` aus `SIMULATION_DURATION` * `TIME_STEPS_PER_SECOND` + 1

**Frame-Speicherung:**

Python liest Simulationsdauer aus `Monitoring.dat`, muss Frame-Frequenz aber aus `C_DELTA`/`C_PLOTFREQ` annehmen. C entscheidet in `fps_check()` (eFlowAgent/PED/ped_param.c:538) welche Frames gespeichert werden:

```c
int fps_check(int loop, REAL delta, REAL fps) {
    int n = max(round(1 / delta / fps), 1);
    if (loop % n == 0) return 1;  // frame speichern
    else return 0;
}
```

Python berechnet identisch (`config.py`:54-55).

`PED.plotfreq` erreicht gewünschte Frequenz nur wenn es Teiler von 1/`PED.delta` ist:
- Bei `PED.delta`=0.1 funktionieren nur `PED.plotfreq`=1,2,5,10 exakt
- `PED.plotfreq`=7 ergibt 10 Hz statt 7 Hz wegen Rundung

**Bei C-Code Änderungen:**
1. `config.py` anpassen (`C_DELTA`, `C_PLOTFREQ`, `GEOMETRY_SIZE`)
2. `TIME_STEPS_PER_SECOND` Neuberechnung prüfen
3. `make clean && make`

Typischer Fehler: `FileNotFoundError` weil C andere `PED.delta`/`PED.plotfreq` verwendet als Python annimmt
- Prüfen welche .dat-Dateien in `Octave/` liegen
- `C_DELTA`/`C_PLOTFREQ` in `config.py` korrigieren

Speicherverschwendung vermeiden:
- `TFRECORD_MAX_STEPS` (aktuell 61) vs `feature_time_steps`/`label_time_steps` (aktuell 50) vergleichen
- Bei großer Differenz `SIMULATION_DURATION` reduzieren

### 5.3. Parameter-Flow: Python -> JSON -> C

`config.py` definiert Parameter -> `create_layout.py` schreibt JSON -> eFlowAgent liest.

Simulation-Parameter:
- `SimulationConfig.MAX_TIME` (= `TimeConfig.SIMULATION_DURATION`) -> `maxTime` -> `PED.MaxTime`
- `SimulationConfig.START_PERSONS` -> `startPersons` -> `PED.Pinit`
- `SimulationConfig.FUNDAMENTAL_DIAGRAM` -> `Fundamentaldiagramm` -> `PED.FD`
- `SimulationConfig.RAND_PERSONS` -> `randPersons` -> `PED.Prand`

Physikalische Skalierung:
- `SimulationConfig.CHAR_LENGTH` -> `charLength` -> `PED.CL`
- `SimulationConfig.CHAR_DENSITY` -> `charDensity` -> `PED.CP`
- `SimulationConfig.CHAR_VELOCITY` -> `charVelocity` -> `PED.CV`

SIR-Infektionsmodell (simuliert, nicht in TFRecords gespeichert):
- `SimulationConfig.PERCENT_INFECTED` -> `percentInfected` -> `siragent.PI`
- `SimulationConfig.CRITICAL_DISTANCE` -> `criticalDistance` -> `siragent.CD`
- `SimulationConfig.INFECTION_RATE` -> `infectionRate` -> `siragent.IR`
- `SimulationConfig.RESISTANCE_TIME` -> `resistanceTime` -> `siragent.RT`
- `SimulationConfig.PERCENT_REMOVED` -> `percentRemoved` -> `siragent.PR`
- `SimulationConfig.MOVE_MODE` -> `moveMode` -> `video.MoveMode`

Nicht über JSON steuerbar, hardcoded in C (siehe 5.2):
- `PED.delta`, `PED.plotfreq`, `alphax`, `alphay`


## 6. eFlowAgent

C-Simulation für Personenströme mit Eikonal-Gleichungen.

### 6.1. Dateien

- **pFlowSIM** - Hauptprogramm, liest JSON und startet Simulation
- **PED/** - Personenstrom-Solver mit Eikonal-Gleichungen
- **Graphics/** - Datenausgabe als .dat-Dateien
- **VideoCreator/** - MATLAB/Octave Visualisierung

### 6.2. Ausführung

```bash
cd eFlowAgent
make clean && make
./pFlowSIM  # Direkte Ausführung (für Tests)
```

Wird automatisch von `DataGeneration.py` aufgerufen.
Liest `00Input/eFlow.dat` für JSON-Pfad mit Layout und Parametern.

### 6.3. Ausgaben

Octave/ Verzeichnis:
- `*_x_0000000.dat`, `*_y_0000000.dat` - Grid-Koordinaten (nur einmal)
- `Rho_z_XXXXXXX.dat` - Personendichte pro Zeitschritt
- `Vel_z_XXXXXXX.dat` - Geschwindigkeit (64×128 Array: vx und vy nebeneinander)
- `Phi_z_XXXXXXX.dat` - Potentialfeld pro Zeitschritt

Namenskonvention: Suffixe `_x`, `_y`, `_z` sind keine 3D-Koordinaten
- `_x` und `_y` = Grid-Koordinaten
- `_z` = Datenwerte

`DataGeneration.py` löscht alte .dat-Dateien automatisch vor jeder Simulation.

### 6.4. Details

Über JSON steuerbar: Siehe Abschnitt 5.3 (Parameter-Flow)

Hardcoded in C (values.c):
- `PED.delta = 0.1`: Zeitschritt (muss mit `config.py` synchron sein, siehe 5.2)
- `PED.plotfreq = 10`: Output-Frequenz (muss mit `config.py` synchron sein, siehe 5.2)
- `PED.empty = 1`: Stoppt wenn <1% Personen übrig

Hardcoded in C (octaveplot.c):
- `alphax = 1./64.`: Räumliche Auflösung (muss mit `config.py` synchron sein, siehe 5.2)

MATLAB/Octave Visualisierung (nur bei manueller Ausführung):
- `VideoTrack.m`: Animierte Evakuierung
- `Monitoring.m`: Personenanzahl über Zeit

## 7. generate

Datengenerierung für SurrogateModel-Training.

### 7.1. Dateien

- **DataGeneration.py**: Hauptskript, steuert Ablauf
- **config.py**: Zentrale Konfiguration (siehe Abschnitt 5)
- **create_layout.py**: Layout-Generator mit rekursiver Raumaufteilung
- **DataGeneration_json.py**: Hilfsfunktionen für Geometrie-Verarbeitung
- **DataGeneration_tfrecord.py**: Konvertiert .dat zu TFRecord
- **split_tfrecords.py**: Splittet in Train/Val/Test (70/20/10)
- **visualize_layouts.py**: PNG-Visualisierung der Layouts

### 7.2. Ausführung

```bash
cd generate
python DataGeneration.py
```

Ablauf:
1. Generiert Layouts und speichert als JSON
2. Ruft eFlowAgent auf (liest JSON) -> .dat-Dateien
3. Konvertiert .dat zu TFRecords
4. Visualisiert erste 20 Layouts als PNG

### 7.3. Ausgaben

data/runs/run_YYYYMMDD_HHMMSS/:
- `layouts/*.json`: Gebäudegeometrien
- `layouts/layout_images/`: PNG-Visualisierungen
- `tfrecords/*.tfrecord`: Trainingsdaten (rho, velocity, phi normalisiert auf [0,1])
- `config.json`: Verwendete Parameter
- `statistics.json`: Laufzeitstatistiken

### 7.4. Details

Alle Parameter in `config.py` (Abschnitt 5).

Normalisierung (DataGeneration_tfrecord.py:112-135):
- MinMax-Scaling auf [0,1]
- Ranges hardcoded: rho [0, 0.25], velocity [0, 1.34], phi [0, 1.0]

Retry-Logik:
- MAX_LAYOUT_ATTEMPTS=20: Bei Fehlschlag neue Positionen und Größen (Türen, Hindernisse) aus Ranges
- MAX_OBSTACLE_ATTEMPTS=2000: Platzierungsversuche pro Raum
- Häufiger Grund für Fehlschläge: Zu viele Hindernisse für kleine Räume

Layout-Visualisierung: `DataGeneration.py` macht automatisch erste 20. Für mehr:
```bash
python visualize_layouts.py  # neuester run, alle layouts
python visualize_layouts.py ../data/runs/run_20250815_141155/layouts/  # spezifischer run
python visualize_layouts.py ../data/runs/run_20250815_141155/layouts/ 50  # erste 50
```

Koordinatensystem: eFlowAgent verwendet mathematische Koordinaten (Y nach oben), Visualisierung Bildkoordinaten (Y nach unten). Transformation (`Y_TRANSFORM_OFFSET` - y) nur in `visualize_layouts.py`, JSON-Dateien unverändert.

## 8. SurrogateModel

ConvLSTM-U-Net für schnelle Vorhersagen.

### 8.1. Dateien

- **main.py** - Training und Evaluation
- **ConvLSTM_UNet_model.py** - U-Net Architektur mit ConvLSTM
- **ConvLSTM_UNet_config.py** - Hyperparameter
- **Dataset.py** - TFRecord Loading und Preprocessing
- **visualization_utils.py** - Plots und Summary Erstellung
- **get_latest_run_paths.py** - Findet neuesten Run automatisch

### 8.2. Training starten

```bash
cd SurrogateModel
python main.py  # Nutzt neuesten Run
```

- Automatischer 70/20/10 Split (Train/Val/Test)
- Für anderen Run: `main.py`:311 ändern von `run_name = None` auf z.B. `run_name = "run_20250815_141155"`

### 8.3. Ausgaben

model_runs/ Verzeichnis:
- `*.keras` - Trainiertes Modell
- `*_model.png` - Architektur (braucht Graphviz)
- `*_loss.png` - Training/Validation Loss
- `*_BSP*.png` - Beispiel-Vorhersagen

### 8.4. Details

Run-Auswahl (main.py:311):
- `run_name = None` verwendet neuesten Run
- `run_name = "run_20250815_141155"` für spezifischen Run

Zeitschritte (main.py:344-349):
- `time_stride = 1`: Nimmt jeden N-ten Frame
- `time_stride_random = False`: Zufälliger Offset pro Sample
- `feature_time_steps = 50`: Input frames 0-49
- `label_time_steps = 50`: Output frames 0-49
- `pred_steps = 1`, `initial_step_pred = 5`
- `feature_time_steps`/`label_time_steps` muss <= `TFRECORD_MAX_STEPS` sein (61 bei 6s Simulation)

Dataset (Dataset.py:129,131,159-160):
- `take_exp = [-1, -1, -1]`: Samples [Test, Train, Val], -1 = alle
- `shuffle = [False, True, False]`: Nur Training shuffled
- `buffer_size`: Dynamisch = `dataset_size`
- `size_geometry = (64, 64)` (main.py:335): Muss mit `config.py`:`GEOMETRY_SIZE` übereinstimmen

Training:
- Optimizer (main.py:353-358, ConvLSTM_UNet_config.py:30): learning_rate=0.001, beta_1=0.9, beta_2=0.999, clipnorm=1.0
- Loss (main.py:355): MSE, loss_weights=[1,1] für [rho, velocity]
- Callbacks (main.py:244-246): EarlyStopping patience=5 ab epoch 10, ReduceLROnPlateau factor=0.1 patience=2
- Epochen (main.py:378): 200
- Batch Size (main.py:85-90): GPU 5*len(gpus), TPU 2*num_replicas, CPU 5
- Graph-Mode: run_eagerly=False (schnell), für Debug run_eagerly=True

Architektur (main.py:365-377):
- U-Net: block_numb=3, feats=5, kernel_size=(3,3), pool_size=(1,2,2), padding='same'
- Aktivierung: tanh in ConvLSTM und Output (main.py:359,370)
- Regularisierung (main.py:360-363): reg_val=0.00000001, drop_val=0.0, drop_val_recur=0.0, batch_norm=False
- return_sequence=True, input=['rho','v','geometry'], output=['rho','v']
- MultiEncoder=False, MultiDecoder=True, Multistep=False
- Phi in TFRecords gespeichert aber nicht verwendet (Dataset.py:175,182,189)

## 9. Bekannte Probleme

### 9.1. Loss-Funktion

Aktuell MSE für beide outputs, gewichtung [1,1] (main.py:355).
- Rho (0-0.25) und velocity (0-1.34) unterschiedlich normalisiert, velocity hat größere Werte
- Tuning-code mit cosine_similarity existiert (main.py:290), unvollständig

Optionen:
- my_loss aktivieren (ConvLSTM_UNet_config.py:96) für relativen Fehler
- Gewichte variieren
- Andere loss functions (z.B. MAE, Huber, Log-Cosh)
- Velocity aufteilen in Betrag + Richtung
- StandardScaler statt MinMax

### 9.2. Hyperparameter und Skalierung
- Hyperparameter des SurrogateModels müssen optimiert werden 
  - An Hardware und Datenmenge anpassen
- Größere Datenmengen testen
- Länger Simulationen testen

### 9.3. Zeitauflösung

10 FPS (Frame alle 0.1s) bei `PED.plotfreq`=10, `PED.delta`=0.1.
- Frames ändern sich wenig zwischen Zeitschritten
- Aktuell `time_stride`=1 (main.py:344) verwendet jeden Frame
- `fps_check()` (eFlowAgent/PED/ped_param.c:538) speichert jeden Schritt

Optionen:
- `time_stride` erhöhen: Flexibel, keine C-Änderung, aber alle 61 Frames bleiben gespeichert
- `PED.plotfreq` in C reduzieren: Weniger Speicher, hardcoded und unflexibel, Berechnung läuft weiter alle 0.1s

### 9.4. Memory-Padding

Rho/Phi (64x64) werden auf 64x128 gepaddet wegen velocity (vx,vy Komponenten).
- DataGeneration.py:123,146,156
- Verbraucht doppelten Speicher für rho/phi
- Padding entfernen und Arrays separat handhaben wäre sparsamer

### 9.5. Input = Output Training

Model trainiert aktuell Rekonstruktion statt Vorhersage.

**Parameter:**
- main.py:346-347: `feature_time_steps = 50`, `label_time_steps = 50`
- Dataset.py:11: `start_time = 0`
- TFRecords: 61 frames (0-60) aus 6s Simulation

**Code-Analyse Dataset.py:305-345 (preprocess):**

Zwei If-Abfragen für Frame-Shift:
```python
# Line 315-318: Feature extraction
feature_rho, feature_v = change_size_startfield(..., start_time=0)
# -> Input frames 0-49

# Line 320-321: Erste If
if label_time_steps - feature_time_steps > 0:  # 50-50=0 -> FALSE
    start_time = start_time + feature_time_steps

# Line 325: Label extraction
label_rho, label_v, label_phi = change_size_label(..., start_time=0)
# -> Output frames 0-49

# Line 343-344: Zweite If
if label_time_steps - feature_time_steps > 0:  # 50-50=0 -> FALSE
    label_time_steps = label_time_steps - feature_time_steps
```

Bei 50=50 triggern beide If-Abfragen nicht. Input = Output = frames 0-49.

**Original Design:**

Code war für `feature_time_steps != label_time_steps` designed.

Beispiel mit `feature_time_steps`=5, `label_time_steps`=50:
- Line 315: Input frames 0-4
- Line 320 triggert: `start_time` = 0 + 5 = 5
- Line 325: Output frames 5-54
- Line 343 triggert: `label_time_steps` = 50 - 5 = 45
- Result: Input 0-4, Output 5-49

**Problem:**

Training: Model sieht frame t -> frame t (lernt Identität)
Inference: Model macht autoregressive Vorhersage (ConvLSTM_UNet_config.py:182-205)
- Start mit 5 frames
- Loop 45x: predicted frame wird Input für nächsten
- Nicht trainiert für diese Aufgabe

**Lösungen:**

**1. Unterschiedliche Timesteps (bestehende If-Logik nutzen)**

main.py:346-347 auf `feature_time_steps = 5, label_time_steps = 50` setzen
- Beide If-Abfragen triggern wie designed
- Input 0-4, Output 5-49
- Matched mit inference (`initial_step_pred`=5)

Alternativ `feature_time_steps`=10 oder 20 für mehr zeitlichen Kontext.

Optional: Sliding Window für mehr Samples
- `read_tfrecord` Loop über `start_point`
- Bei `feature_time_steps`=10: Samples mit start 0,5,10,15 aus 61 frames
- 300 layouts -> 1200+ samples

**2. Teacher Forcing (1-Frame Shift)**

Dataset.py:320-321 durch `label_start_time = start_time + 1` ersetzen
Line 343-344 löschen
- Input 0-49, Output 1-50
- Model lernt t -> t+1
- Maximaler Context, aber nur 1-step prediction

### 9.6. C-Python Synchronisation

Parameter in C hardcoded, müssen manuell mit `config.py` synchron gehalten werden (`PED.delta`, `PED.plotfreq`, `alphax`).
- Keine automatische Verifizierung
- Falsche Werte führen zu `FileNotFoundError` oder shape mismatch

Siehe Abschnitt 5.2 für Details. .dat-header parsen statt Annahmen wäre sicherer.

### 9.7. SIR-Daten nicht gespeichert

C simuliert SIR-Modell, speichert aber nur rho/vel/phi in .dat.
- Infektionsstatus S/I/R wird nicht in TFRecords geschrieben
- SIR deaktivieren oder Daten ins Training einbeziehen

### 9.8. Entrance-Feature nicht implementiert

C unterstützt Eingänge (kontinuierlicher Personenstrom), Python-Seite nicht.
- `create_layout.py`:649-650 hardcoded `personPerSecond`=0, `maxPersons`=0
- Config-Parameter fehlen
- Bisher nur Evakuierung getestet

### 9.9. Config-Parameter werden ignoriert

SurrogateModel ignoriert mehrere Parameter aus `generate`/`config.py`:

- main.py:335 hardcoded `size_geometry = (64, 64)` statt `DataConfig.GEOMETRY_SIZE`
- Dataset.py:156 hardcoded `SHARD_SIZE = 5`
- Dataset.py:315,325 geben `size_geometry` nicht an `change_size_startfield`/`change_size_label` weiter
- DataGeneration_tfrecord.py:187 hardcoded `> 500` statt `TimeConfig.TFRECORD_MAX_STEPS`
- DataGeneration_tfrecord.py:217,232 hardcoded 5 in `shape=(4, 5)` statt `MAX_EXIT_POINTS`

Bei Änderung von `config.py` = shape mismatch oder falsche buffer size

### 9.10. Code-Struktur

Mehrere Python-Module haben sehr lange Funktionen und Code-Duplikate. Refactoring in kleinere Funktionen wäre wartbarer und besser lesbar.



