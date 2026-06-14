# Datengenerierung
class DataConfig:
    # datengenerierung parameter

    NUM_LAYOUTS = 1000              # anzahl layouts
    GEOMETRY_SIZE = (64, 64)      # muss mit c übereinstimmen (alphax=1./64.)
    SHARD_SIZE = 5                # layouts pro tfrecord
    MAX_LAYOUT_ATTEMPTS = 20      # versuche mit neuen positionen/größen aus ranges
    VISUALIZE_LAYOUTS = 20        # max visualisierte layouts


# Layout-Generierung
class LayoutConfig:
    # layout generierung

    # gebäude komponenten (min, max)
    NUM_ROOMS = (1, 5)              # räume pro layout
    NUM_EXITS = (1, 2)              # ausgänge
    NUM_ENTRANCES = (0, 0)          # auf 0 lassen, c-support da aber python fehlt (create_layout.py:652-653 hardcoded 0)
    NUM_OBSTACLES = (0, 4)          # hindernisse

    # größen in pixel (min, max)
    ROOM_SIZE_RANGE = (150, 450)        # raumgröße
    OBSTACLE_SIZE_RANGE = (50, 450)    # hindernisgröße
    DOOR_WIDTH_RANGE = (30, 120)        # türbreite
    BORDER_DISTANCE_RANGE = (5, 30)     # abstand zum rand

    # variabilität
    ROOM_POSITION_VARIANCE_X = 5    # teiler für x-verschiebung (größer = weniger variation)
    ROOM_POSITION_VARIANCE_Y = 4    # teiler für y-verschiebung (größer = weniger variation)

    # constraints
    MIN_ROOM_SIZE_LIMIT = 50        # min raumgröße
    MIN_DOOR_WIDTH = 10             # min türbreite
    MIN_WALL_DISTANCE = 40          # hindernis abstand zu wand
    MAX_OBSTACLE_ATTEMPTS = 2000    # platzierungsversuche pro raum beim hindernis-platzieren
    ALLOW_OVERLAP = False           # überlappung erlaubt?

    # canvas
    DEFAULT_MAX_BUILDING_SIZE = (1000, 1000)  # pixel-koordinatensystem (physikalische größe via CHAR_LENGTH)


# Zeit-Parameter
class TimeConfig:
    # zeit-sync zwischen c und python
    # c_delta und c_plotfreq sind annahmen über c code

    # annahmen über c simulation (base/values.c)
    C_DELTA = 0.1             # sollte PED.delta in c sein
    C_PLOTFREQ = 10           # sollte PED.plotfreq in c sein

    # daraus berechnet (implementiert fps_check formel aus c)
    # n = max(round(1/delta/fps), 1) aus eFlowAgent/PED/ped_param.c:541
    n = max(round(1 / C_DELTA / C_PLOTFREQ), 1)
    TIME_STEPS_PER_SECOND = int((1 / C_DELTA) / n)  # tatsächliche output-rate

    # simulationsdauer
    SIMULATION_DURATION = 6    # sekunden

    # array-größe für tfrecords (alle arrays werden auf diese länge gepaddet)
    TFRECORD_MAX_STEPS = SIMULATION_DURATION * TIME_STEPS_PER_SECOND + 1  # = anzahl generierter frames
    # siehe SurrogateModel/main.py:350-351 für tatsächliche verwendung


# Simulations-Parameter (Python -> JSON -> C)
class SimulationConfig:
    # parameter für c simulation (über json)

    # physik skalierung
    CHAR_LENGTH = 28            # gebäudegröße in metern (1000px canvas = CHAR_LENGTH meter)
    CHAR_DENSITY = 8            # 1.0 = 8 personen/m² (int)
    CHAR_VELOCITY = 1.34        # 1.0 = 1.34 m/s

    # evakuierung
    START_PERSONS = 100         # anzahl personen (int)
    MAX_TIME = TimeConfig.SIMULATION_DURATION  # dauer [s]
    FUNDAMENTAL_DIAGRAM = 5     # bewegungsmodell

    # sir infektionsmodell
    PERCENT_INFECTED = 0.05     # initial infiziert
    CRITICAL_DISTANCE = 0.5     # ansteckung distanz [m]
    INFECTION_RATE = 65         # infektionsrate (int)
    PERCENT_REMOVED = 0.1       # initial immun
    RESISTANCE_TIME = 1         # resistenz [s] (int)

    # sonstiges
    RAND_PERSONS = 0             # 0=gleichmäßig, 1=random
    MOVE_MODE = 0                # wird gelesen aber nie verwendet


# Technische Konstanten
class TechnicalConstants:

    # verzeichnisse
    INPUT_DIR = "00Input"
    OUTPUT_DIR = "00Output"
    OCTAVE_DIR = "Octave"
    CONFIG_FILENAME = "eFlow.dat"
    MONITORING_FILENAME = "Monitoring.dat"

    # farbcodes
    WALL_COLOR = 0
    FLOOR_COLOR = 255
    EXIT_COLOR = 1
    ENTRANCE_COLOR = 2
    OBSTACLE_COLOR = 4
    CORRIDOR_COLOR = 6

    # datenformat
    TFRECORD_PATTERN = "pFlow-{shard:05d}-of-{total:05d}.tfrecord"
    MAX_EXIT_POINTS = 5         # hardcoded im tfrecord schema

    # y-koordinaten transform
    Y_TRANSFORM_OFFSET = 1000   # bild zu mathe koordinaten

    # diverses
    SIMULATION_TIMEOUT = 3600    # timeout für pflowsim [s]
    TIMESTAMP_DIGITS = 7         # zeitstempel format
    LAYOUT_ID_SEED = 42          # seed für layout ids