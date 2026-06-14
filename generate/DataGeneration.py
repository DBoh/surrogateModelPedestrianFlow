import random
import os
from pathlib import Path
from datetime import datetime
import json
import subprocess
from collections import namedtuple
import logging

import numpy as np
import DataGeneration_tfrecord as DG_tfr
import DataGeneration_json as DG_js
import create_layout as create_layout
from config import TechnicalConstants, TimeConfig, DataConfig, LayoutConfig


logger = logging.getLogger(__name__)


# layout parameter
LayoutParams = namedtuple('LayoutParams', [
    'num_rooms',
    'num_exits', 
    'num_entrances',
    'num_obstacles',
    'min_room_size',
    'obstacle_size',
    'door_width'
])


def get_project_root():
    current_file = Path(__file__).resolve()
    return current_file.parent.parent


def exec_simulation(config_file, json_file):
    import time
    labels = []
    project_root = get_project_root()
    eflow_dir = os.path.join(project_root, "eFlowAgent")
    
    # verzeichnisse erstellen
    os.makedirs(os.path.join(eflow_dir, TechnicalConstants.INPUT_DIR), exist_ok=True)
    os.makedirs(os.path.join(eflow_dir, TechnicalConstants.OUTPUT_DIR), exist_ok=True)
    os.makedirs(os.path.join(eflow_dir, TechnicalConstants.OCTAVE_DIR), exist_ok=True)
    
    # alte octave dateien löschen
    octave_dir = os.path.join(eflow_dir, TechnicalConstants.OCTAVE_DIR)
    for filename in os.listdir(octave_dir):
        if filename.endswith('.dat') or filename.endswith('.m'):
            file_path = os.path.join(octave_dir, filename)
            try:
                if os.path.isfile(file_path):
                    os.remove(file_path)
            except (OSError, PermissionError) as e:
                logger.warning(f"konnte {file_path} nicht löschen: {e}")
    
    # absoluter pfad für json
    json_abs_path = os.path.abspath(json_file)
    
    # config schreiben
    try:
        rel_path = os.path.relpath(json_abs_path, eflow_dir)
        with open(config_file, 'w') as file:
            file.write(rel_path)
    except ValueError:
        # falls relativ nicht geht
        with open(config_file, 'w') as file:
            file.write(json_abs_path)
    
    # Führe Simulation aus
    pflow_path = os.path.join(eflow_dir, "pFlowSIM")
    
    if not os.path.exists(pflow_path):
        logger.error(f"pflowsim nicht da: {pflow_path}")
        return -1
    
    try:
        # Zeitmessung starten
        start_time = time.time()
        
        result = subprocess.run(
            [pflow_path],
            cwd=eflow_dir,
            capture_output=True,
            text=True,
            timeout=TechnicalConstants.SIMULATION_TIMEOUT
        )
        
        # Berechne Ausführungszeit
        calculation_time = time.time() - start_time
        
        if result.returncode != 0:
            # fehler
            return result.returncode
        
        # Lese Ergebnisse
        monitoring_file = os.path.join(eflow_dir, TechnicalConstants.OUTPUT_DIR, "Monitoring.dat")
        if not os.path.exists(monitoring_file):
            return -996  # monitoring.dat fehlt
        
        with open(monitoring_file, "r") as file:
            lines = file.readlines()
            if not lines:
                return -995  # monitoring.dat leer
            max_time = float(lines[-1].strip().split()[0])
        
        # daten sammeln
        labels = []
        
        for i in range(int(abs(max_time) * TimeConfig.TIME_STEPS_PER_SECOND + 1)):
            timestamp = str(i).zfill(TechnicalConstants.TIMESTAMP_DIGITS)
            
            # Lese Rho_z
            rho_file = os.path.join(octave_dir, f'Rho_z_{timestamp}.dat')
            try:
                with open(rho_file, "r") as file:
                    data_list = [line.strip().split() for line in file]
                rho_z = np.array(data_list, dtype=float)
                
                # padding
                rho_z = np.pad(rho_z, ((0, 0), (0, rho_z.shape[1])), constant_values=0.0)
                
            except FileNotFoundError:
                break
            
            # Lese Vel_z
            vel_file = os.path.join(octave_dir, f'Vel_z_{timestamp}.dat')
            try:
                with open(vel_file, "r") as file:
                    data_list = [line.strip().split() for line in file]
                vel_z = np.array(data_list, dtype=float)
                # kein padding bei vel
            except FileNotFoundError:
                vel_z = np.zeros_like(rho_z[:, :rho_z.shape[1]//2])  # halbe breite
            
            # Lese Phi_z
            phi_file = os.path.join(octave_dir, f'Phi_z_{timestamp}.dat')
            try:
                with open(phi_file, "r") as file:
                    data_list = [line.strip().split() for line in file]
                phi_z = np.array(data_list, dtype=float)
                
                # Padding mit np.pad
                phi_z = np.pad(phi_z, ((0, 0), (0, phi_z.shape[1])), constant_values=0.0)
                
            except FileNotFoundError:
                # vorheriger zeitschritt
                if i > 0:
                    prev_phi_file = os.path.join(octave_dir, f'Phi_z_{str(i-1).zfill(TechnicalConstants.TIMESTAMP_DIGITS)}.dat')
                    try:
                        with open(prev_phi_file, "r") as file:
                            data_list = [line.strip().split() for line in file]
                        phi_z = np.array(data_list, dtype=float)
                        phi_z = np.pad(phi_z, ((0, 0), (0, phi_z.shape[1])), constant_values=0.0)
                    except FileNotFoundError:
                        phi_z = np.zeros_like(rho_z)
                else:
                    phi_z = np.zeros_like(rho_z)
            
            labels.append([rho_z, vel_z, phi_z])
        
        # Lese Positionen
        position_x_file = os.path.join(octave_dir, 'Rho_x_0000000.dat')
        position_y_file = os.path.join(octave_dir, 'Rho_y_0000000.dat')
        
        with open(position_x_file, "r") as file:
            position_x = [line.strip() for line in file]
        with open(position_y_file, "r") as file:
            position_y = [line.strip() for line in file]
        
        position = [[np.array([x, y]) for y in position_y] for x in position_x]
        
        return labels, position, max_time, calculation_time
        
    except subprocess.TimeoutExpired:
        return -999  # timeout
    except (OSError, subprocess.SubprocessError) as e:
        logger.debug(f"pflowsim fehler: {str(e)}")
        return -998  # ausführungsfehler
    except Exception as e:
        logger.debug(f"unerwarteter fehler: {str(e)}")
        return -997


def generate_parameter_list_generated_synthesized_data(num_data, num_rooms, num_exits, num_entrances, 
                                                      num_obstacles, min_room_size, obstacle_size, 
                                                      door_width):
    parameterlist = []
    for _ in range(num_data):
        params = LayoutParams(
            num_rooms=random.randint(num_rooms[0], num_rooms[1]),
            num_exits=random.randint(num_exits[0], num_exits[1]),
            num_entrances=random.randint(num_entrances[0], num_entrances[1]),
            num_obstacles=random.randint(num_obstacles[0], num_obstacles[1]),
            min_room_size=random.randint(min_room_size[0], min_room_size[1]),
            obstacle_size=obstacle_size,
            door_width=door_width
        )
        parameterlist.append(params)
    return parameterlist


def correct_dimension(dens, vel, start_pers, fd, exit_points, rho_start, vel_start, geometry_im, position, time, label, analyse_param,
                      max_exit_points=TechnicalConstants.MAX_EXIT_POINTS, max_timesteps=TimeConfig.TFRECORD_MAX_STEPS):
    while len(exit_points) < max_exit_points:
        exit_points.append([0, 0, 0, 0])
    if len(exit_points) > max_exit_points:
        logger.warning(f"exit_points: zu viele Elemente ({len(exit_points)} / {max_exit_points})")
    
    # Konvertiere label zu Liste falls es ein NumPy Array ist
    if isinstance(label, np.ndarray):
        label = label.tolist()
    
    while len(label) < max_timesteps:
        label.append(label[-1])
    if len(label) > max_timesteps:
        logger.warning(f"label: Sehr lange Simulation ({len(label)} Zeitschritte / max {max_timesteps})")
    return (dens, vel, start_pers, fd, exit_points, rho_start, vel_start, geometry_im, position, time, label,
            analyse_param)


def add_tfrecord(generate_synthesized_data, size_shard, size_geometry, allow_overlap,
               num_rooms, num_exits, num_entrances, num_obstacles,
               min_room_size, obstacle_size, door_width, path=None, tfr_path=None):
    
    project_root = get_project_root()
    
    # run verzeichnis
    if generate_synthesized_data > 0:
        # nur "run" verwenden, pflowsim mag kein camelCase
        run_name = f"run_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        run_dir = os.path.join(project_root, "data", "runs", run_name)
        os.makedirs(run_dir, exist_ok=True)
        
        path = os.path.join(run_dir, "layouts")
        tfr_path = os.path.join(run_dir, "tfrecords")
        os.makedirs(path, exist_ok=True)
        os.makedirs(tfr_path, exist_ok=True)
        
        # config speichern
        config = {
            "run_name": run_name,
            "timestamp": datetime.now().isoformat(),
            "parameters": {
                "generate_synthesized_data": generate_synthesized_data,
                "size_geometry": size_geometry,
                "size_shard": size_shard,
                "num_rooms": num_rooms,
                "num_exits": num_exits,
                "num_obstacles": num_obstacles,
                "min_room_size": min_room_size,
                "obstacle_size": obstacle_size,
                "door_width": door_width,
                "allow_overlap": allow_overlap
            }
        }
        
        with open(os.path.join(run_dir, "config.json"), 'w') as f:
            json.dump(config, f, indent=4)
        
        logger.info(f"run: {run_name}")
    
    # stats
    stats = {
        'total_layouts': 0,
        'successful_layouts': 0,
        'failed_layouts': 0,
        'total_simulations': 0,
        'successful_simulations': 0,
        'failed_simulations': 0,
        'total_tfrecords': 0,
        'layout_attempts': 0,
        'simulation_times': [],
        'avg_simulation_time': 0.0,
        'min_simulation_time': float('inf'),
        'max_simulation_time': 0.0
    }
    
    # layout generierung
    if generate_synthesized_data > 0:
        logger.info(f"\ngeneriere {generate_synthesized_data} layouts")
        logger.info(f"Parameter-Ranges:")
        logger.info(f"  Räume: {num_rooms}")
        logger.info(f"  Ausgänge: {num_exits}")
        logger.info(f"  Hindernisse: {num_obstacles}")
        logger.info(f"  Hindernisgröße: {obstacle_size}px")
        logger.info(f"  Raumgröße: {min_room_size}px")
        logger.info(f"Retry-Logik:")
        logger.info(f"  Bei Fehlschlag neue Positionen/Größen aus Ranges: max {DataConfig.MAX_LAYOUT_ATTEMPTS}")
        logger.info(f"  Pro Raum beim Hindernis-Platzieren: max {LayoutConfig.MAX_OBSTACLE_ATTEMPTS}")
        
        param = generate_parameter_list_generated_synthesized_data(
            generate_synthesized_data, num_rooms, num_exits, num_entrances, 
            num_obstacles, min_room_size, obstacle_size, door_width
        )
        
        json_files = []
        max_placement_attempts = DataConfig.MAX_LAYOUT_ATTEMPTS
        
        # generiere layouts mit zufaellig gewaehlten parametern aus den ranges
        for i, p in enumerate(param):
            stats['total_layouts'] += 1
            layout_created = False
            
            logger.info(f"\nLayout {i+1}/{generate_synthesized_data} (Parameter: {p.num_rooms} Räume, {p.num_obstacles} Hindernisse)")
            
            for attempt in range(max_placement_attempts):
                stats['layout_attempts'] += 1
                try:
                    layout = create_layout.RoomLayout()
                    created = layout.create_polygon_layout(
                        num_rooms=p.num_rooms,
                        num_exits=p.num_exits,
                        num_entrance=p.num_entrances, 
                        num_obstacles=p.num_obstacles,
                        min_room_size=p.min_room_size,
                        obstacle_size=p.obstacle_size,
                        door_width=p.door_width
                    )
                    
                    if created and len(created[0]) > 0:
                        json_path = os.path.join(path, f"generated_{layout.generate_id(TechnicalConstants.LAYOUT_ID_SEED)}.json")
                        layout.create_json(json_path)
                        json_files.append(json_path)
                        stats['successful_layouts'] += 1
                        logger.info(f"  Erfolgreich beim {attempt+1}. Versuch")
                        layout_created = True
                        break
                    else:
                        if attempt < max_placement_attempts - 1:
                            reason = create_layout.RoomLayout.last_error_reason or "unbekannter fehler"
                            logger.info(f"  Versuch {attempt+1} fehlgeschlagen: {reason}")
                except (ValueError, RuntimeError) as e:
                    if attempt < max_placement_attempts - 1:
                        logger.info(f"  Versuch {attempt+1} fehlgeschlagen: {str(e)}")
            
            if not layout_created:
                stats['failed_layouts'] += 1
                reason = create_layout.RoomLayout.last_error_reason or "alle versuche fehlgeschlagen"
                logger.warning(f"  Fehlgeschlagen nach {max_placement_attempts} Versuchen. Letzter Grund: {reason}")
        
        if stats['failed_layouts'] > 0:
            failure_rate = stats['failed_layouts'] / stats['total_layouts']
            logger.info(f"\nLayout-Generierung abgeschlossen:")
            logger.info(f"  Erfolgreich: {stats['successful_layouts']}/{stats['total_layouts']}")
            logger.info(f"  Fehlgeschlagen: {stats['failed_layouts']} ({failure_rate:.1%})")
            logger.info(f"  Gesamtversuche: {stats['layout_attempts']}")
            if create_layout.RoomLayout.layouts_rejected > 0:
                logger.info(f"  Verworfene Layouts (insgesamt): {create_layout.RoomLayout.layouts_rejected}")
    
    # json dateien sammeln
    files = []
    if path and os.path.isdir(path):
        for f in os.listdir(path):
            if f.endswith('.json'):
                files.append(os.path.join(path, f))
    
    if not files:
        error_msg = "keine json dateien gefunden"
        logger.error(error_msg)
        if generate_synthesized_data > 0:
            stats_file = os.path.join(run_dir, "statistics.json")
            with open(stats_file, 'w') as f:
                json.dump(stats, f, indent=4)
            logger.error(f"keine layouts generiert")
        raise ValueError(error_msg)
    
    # simulationen
    logger.info(f"\nsimuliere {len(files)} layouts")
    
    data_set = []
    shard = 0
    
    for i, json_file in enumerate(files):
        stats['total_simulations'] += 1
        
        try:
            dens, vel, start_pers, fd, exit_points, geometry_im, analyse_param = DG_js.read_json(json_file, size_geometry)
            
            config_file = os.path.join(project_root, "eFlowAgent", TechnicalConstants.INPUT_DIR, TechnicalConstants.CONFIG_FILENAME)
            result = exec_simulation(config_file, json_file)
            
            if result != -1:
                labels, position, time, calculation_time = result
                rho_start = labels[0][0]
                vel_start = labels[0][1]
                analyse_param['calculation_time_eFlow'] = float(calculation_time)
                
                data_entry = correct_dimension(
                    dens, vel, start_pers, fd, exit_points, rho_start, vel_start, 
                    geometry_im, position, time, labels, analyse_param
                )
                
                data_set.append(data_entry)
                stats['successful_simulations'] += 1
                logger.info(f"Simulation {i+1}/{len(files)}: {os.path.basename(json_file)} ... OK ({calculation_time:.1f}s)")
                
                # Aktualisiere Zeitstatistiken
                stats['simulation_times'].append(calculation_time)
                stats['min_simulation_time'] = min(stats['min_simulation_time'], calculation_time)
                stats['max_simulation_time'] = max(stats['max_simulation_time'], calculation_time)
                
                # Schreibe TFRecord-Shard wenn voll
                if len(data_set) >= size_shard:
                    total_shards = (len(files) + size_shard - 1) // size_shard
                    filename = f"pFlow-{shard:05d}-of-{total_shards-1:05d}.tfrecord"
                    tfrecords_shard_path = os.path.join(tfr_path, filename)
                    
                    DG_tfr.write_features_tfrecord(data_set, tfrecords_shard_path)
                    logger.info(f"TFRecord geschrieben: {filename} ({len(data_set)} Samples)")
                    
                    data_set = []
                    shard += 1
                    stats['total_tfrecords'] += 1
                    
            else:
                stats['failed_simulations'] += 1
                # fehler
                if time == -999:
                    error_msg = "timeout"
                elif time < 0:
                    error_msg = f"code {time}"
                else:
                    error_msg = "fehler"
                logger.warning(f"Simulation {i+1}/{len(files)}: {os.path.basename(json_file)} ... FEHLER ({error_msg})")
                
        except (IOError, ValueError, KeyError) as e:
            stats['failed_simulations'] += 1
            logger.warning(f"Simulation {i+1}/{len(files)}: Fehler beim Lesen - {str(e)}")
        except Exception as e:
            stats['failed_simulations'] += 1
            logger.error(f"Simulation {i+1}/{len(files)}: Unerwarteter Fehler - {str(e)}")
    
    # letzter shard
    if data_set:
        total_shards = (len(files) + size_shard - 1) // size_shard
        filename = f"pFlow-{shard:05d}-of-{total_shards-1:05d}.tfrecord"
        tfrecords_shard_path = os.path.join(tfr_path, filename)
        
        DG_tfr.write_features_tfrecord(data_set, tfrecords_shard_path)
        logger.info(f"TFRecord geschrieben: {filename} ({len(data_set)} Samples)")
        stats['total_tfrecords'] += 1
    
    if stats['simulation_times']:
        stats['avg_simulation_time'] = sum(stats['simulation_times']) / len(stats['simulation_times'])
    
    # stats speichern
    if generate_synthesized_data > 0:
        if stats['min_simulation_time'] == float('inf'):
            stats['min_simulation_time'] = 0.0
            
        stats_file = os.path.join(run_dir, "statistics.json")
        with open(stats_file, 'w') as f:
            json.dump(stats, f, indent=4)
    
    # zusammenfassung
    logger.info(f"\n--- ergebnisse ---")
    logger.info(f"Layouts:")
    logger.info(f"  Generiert: {stats['successful_layouts']}/{stats['total_layouts']}")
    if stats['failed_layouts'] > 0:
        logger.info(f"  Fehlgeschlagen: {stats['failed_layouts']}")
    logger.info(f"\nSimulationen:")
    logger.info(f"  Erfolgreich: {stats['successful_simulations']}/{stats['total_simulations']}")
    if stats['failed_simulations'] > 0:
        logger.info(f"  Fehlgeschlagen: {stats['failed_simulations']}")
    if stats['avg_simulation_time'] > 0:
        logger.info(f"  Durchschnittliche Zeit: {stats['avg_simulation_time']:.1f}s")
        logger.info(f"  Min/Max Zeit: {stats['min_simulation_time']:.1f}s / {stats['max_simulation_time']:.1f}s")
    
    if stats['total_tfrecords'] > 0:
        logger.info(f"\nTFRecords: {stats['total_tfrecords']} Dateien geschrieben")
    
    if stats['successful_simulations'] == 0:
        logger.error("\nkeine simulationen erfolgreich")
    else:
        logger.info(f"\nFertig.")
    
    # visualisierung
    if stats['successful_layouts'] > 0 and path and DataConfig.VISUALIZE_LAYOUTS is not False:
        
        try:
            import visualize_layouts
            visualize_layouts.visualize_all_layouts(path, max_layouts=DataConfig.VISUALIZE_LAYOUTS)
        except ImportError as e:
            logger.debug(f"Visualisierung nicht verfügbar: {e}")
        except Exception as e:
            logger.warning(f"Fehler bei Visualisierung: {e}")


def read_tfrecord(files, take_exp, batch_size, shuffle, time_steps, output_steps, size_geometry):
    if time_steps >= 1 and time_steps > output_steps:
        output_steps = time_steps
    return DG_tfr.read_tfrecord(take_exp, batch_size, shuffle, files, time_steps, output_steps, size_geometry)


def main():
    # Konfiguriere Logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(message)s'
    )
    
    logger.info("Starte Datengenerierung...")
    add_tfrecord(
        generate_synthesized_data=DataConfig.NUM_LAYOUTS,
        size_geometry=DataConfig.GEOMETRY_SIZE,
        size_shard=DataConfig.SHARD_SIZE,
        num_rooms=LayoutConfig.NUM_ROOMS,
        num_exits=LayoutConfig.NUM_EXITS,
        num_entrances=LayoutConfig.NUM_ENTRANCES,
        num_obstacles=LayoutConfig.NUM_OBSTACLES,
        min_room_size=LayoutConfig.ROOM_SIZE_RANGE,
        obstacle_size=LayoutConfig.OBSTACLE_SIZE_RANGE,
        door_width=LayoutConfig.DOOR_WIDTH_RANGE,
        allow_overlap=LayoutConfig.ALLOW_OVERLAP
    )


if __name__ == "__main__":
    main()