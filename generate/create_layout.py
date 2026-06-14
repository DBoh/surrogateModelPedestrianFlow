import json
import string
import random
import numpy as np
from shapely.geometry import LineString, box, MultiPolygon, Polygon
from shapely import ops, get_geometry, union_all, get_type_id
from config import TechnicalConstants, SimulationConfig, LayoutConfig


class RoomLayout:
    layouts_rejected = 0
    last_error_reason = ""
    def __init__(self, char_length=None, char_density=None, char_velocity=None, start_persons=None, rand_persons=None,
                 max_time=None, fd=None, percent_infected=None, critical_distance=None, infection_rate=None,
                 percent_removed=None, resistance_time=None, move_mode=None, decomposition_level="medium", local_refinement=0,
                 local_mark_method=0, global_refinement=0, max_size=None):
        # defaults aus config
        if char_length is None:
            char_length = SimulationConfig.CHAR_LENGTH
        if char_density is None:
            char_density = SimulationConfig.CHAR_DENSITY
        if char_velocity is None:
            char_velocity = SimulationConfig.CHAR_VELOCITY
        if start_persons is None:
            start_persons = SimulationConfig.START_PERSONS
        if max_time is None:
            max_time = SimulationConfig.MAX_TIME
        if fd is None:
            fd = SimulationConfig.FUNDAMENTAL_DIAGRAM
        if percent_infected is None:
            percent_infected = SimulationConfig.PERCENT_INFECTED
        if critical_distance is None:
            critical_distance = SimulationConfig.CRITICAL_DISTANCE
        if infection_rate is None:
            infection_rate = SimulationConfig.INFECTION_RATE
        if percent_removed is None:
            percent_removed = SimulationConfig.PERCENT_REMOVED
        if resistance_time is None:
            resistance_time = SimulationConfig.RESISTANCE_TIME
        if rand_persons is None:
            rand_persons = SimulationConfig.RAND_PERSONS
        if move_mode is None:
            move_mode = SimulationConfig.MOVE_MODE
        if max_size is None:
            max_size = LayoutConfig.DEFAULT_MAX_BUILDING_SIZE
        self.name = "generated"

        self.char_length = char_length
        self.char_density = char_density
        self.char_velocity = char_velocity
        self.start_persons = start_persons
        self.rand_persons = rand_persons
        self.max_time = max_time

        self.fd = fd

        self.percent_infected = percent_infected
        self.critical_distance = critical_distance
        self.infection_rate = infection_rate
        self.percent_removed = percent_removed
        self.resistance_time = resistance_time
        self.move_mode = move_mode

        self.measurementstations = []
        self.entrance = []
        self.exits = []
        self.attractors = []
        self.subdomains_fd = []
        self.subdomains_rho_init = []

        self.decomposition_level = decomposition_level
        self.local_refinement = local_refinement
        self.local_mark_method = local_mark_method
        self.global_refinement = global_refinement

        self.domainpolygon = []
        self.obstacle_geometries = []
        self.max_size = max_size
        self.exit_wall_points = {}

        self.room_width_to_door_width_ratio = []
        self.door_orientation = []
        self.room_shape = []
        self.door_position = []
        self.corridor_length_to_width_ratio = []
        self.building_area_to_room_area_ratio = 0
        self.room_area_to_obstacle_area_ratio = 0
        self.exit_to_room_ratio = []
        self.room_area = []
        self.hallway_area = []
        self.building_area = 0
        self.obstacle_area = []
        self.num_rooms = 0
        self.num_exits = 0
        self.num_entrance = 0
        self.num_obstacles = 0
        self.min_room_size = 0
        self.obstacle_size = 0
        self.door_width = 0


    def print_analysis(self):
        analyse = (f"Analyse:\n"
                   f"Verhältnis Raumbreite zu Türbreite (room_width_to_door_width_ratio):\n{self.room_width_to_door_width_ratio}\n\n" # 1: Tür/Raum gleich breit
                   f"Ausrichtung der Türe im Raum (door_orientation):\n{self.door_orientation}\n\n" # Türe befindet sich auf <1: schmale Seite, >1: breite Seite
                   f"Form des Raums (room_shape):\n{self.room_shape}\n\n" # 1: Rechteckig, <1: gegen 0 immer schmaler und länglicher
                   f"Position der Türen (door_position):\n{self.door_position}\n\n" # 1: mittig von der Wand, gegen 0: außen
                   f"Verhältnis Laenge zur Breite des Flures (corridor_length_to_width_ratio):\n{self.corridor_length_to_width_ratio}\n\n" # 1: Quadratisch, >1: Breite>Länge, <1: Länge>Breite
                   f"Verhältnis der Gebäudefläche zu der Raumfläche (building_to_room_area_ratio):\n{self.building_area_to_room_area_ratio}\n\n" # Raumfläche / Gebäudefläche
                   f"Verhältnis der Raumfläche zu Hindernisse (room_area_to_obstacle_area_ratio):\n{self.room_area_to_obstacle_area_ratio}\n\n" # Hindernissfläche / Raumfläche
                   f"Verhältnis der Ausgänge zu Anzahl der Räume (exit_to_room_ratio):\n{self.exit_to_room_ratio}\n\n" # gleich wie room_width_to_door_width_ratio nur extra für die Ausgänge
                   f"Raumfläche (room_area):\n{self.room_area}\n\n"
                   f"Hindernissfläche (obstacle_area):\n{self.obstacle_area}\n\n"
                   f"Flurfläche (hallway_area):\n{self.hallway_area}\n\n"
                   f"Raum+Flur Fläche:\n{sum(self.hallway_area+self.room_area)}\n\n"
                   f"Gebäudefläche (building_area):\n{self.building_area}\n\n")

        config_param = (f"max_size: {self.max_size}\n"
                        f"num_rooms: {self.num_rooms}\n"
                        f"num_exits: {self.num_exits}\n"
                        f"num_entrance: {self.num_entrance}\n"
                        f"num_obstacles: {self.num_obstacles}\n"
                        f"min_room_size: {self.min_room_size}\n"
                        f"obstacle_size: {self.obstacle_size}\n"
                        f"door_width: {self.door_width}\n\n")

    def create_polygon_layout(self, num_rooms=None, num_exits=None, num_entrance=None, num_obstacles=None, min_room_size=None,
                              obstacle_size=None, door_width=None, print_debug=False, allow_overlap=None):
        # Verwende Standardwerte aus settings wenn nicht angegeben
        if num_rooms is None:
            num_rooms = random.randint(LayoutConfig.NUM_ROOMS[0], LayoutConfig.NUM_ROOMS[1])
        elif isinstance(num_rooms, (tuple, list)):
            num_rooms = random.randint(num_rooms[0], num_rooms[1])
        if num_exits is None:
            num_exits = random.randint(LayoutConfig.NUM_EXITS[0], LayoutConfig.NUM_EXITS[1])
        elif isinstance(num_exits, (tuple, list)):
            num_exits = random.randint(num_exits[0], num_exits[1])
        if num_obstacles is None:
            num_obstacles = random.randint(LayoutConfig.NUM_OBSTACLES[0], LayoutConfig.NUM_OBSTACLES[1])
        elif isinstance(num_obstacles, (tuple, list)):
            num_obstacles = random.randint(num_obstacles[0], num_obstacles[1])
        if num_entrance is None:
            num_entrance = random.randint(LayoutConfig.NUM_ENTRANCES[0], LayoutConfig.NUM_ENTRANCES[1])
        elif isinstance(num_entrance, (tuple, list)):
            num_entrance = random.randint(num_entrance[0], num_entrance[1])
        if min_room_size is None:
            min_room_size = random.randint(LayoutConfig.ROOM_SIZE_RANGE[0], LayoutConfig.ROOM_SIZE_RANGE[1])
        elif isinstance(min_room_size, (tuple, list)):
            # Wenn Range übergeben wurde, zufälligen Wert daraus wählen
            min_room_size = random.randint(min_room_size[0], min_room_size[1])
        if obstacle_size is None:
            obstacle_size = LayoutConfig.OBSTACLE_SIZE_RANGE
        if door_width is None:
            door_width = LayoutConfig.DOOR_WIDTH_RANGE
        if allow_overlap is None:
            allow_overlap = LayoutConfig.ALLOW_OVERLAP
        
        # prüfung raumgröße
        if min_room_size < LayoutConfig.MIN_ROOM_SIZE_LIMIT:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = f"raum zu klein: {min_room_size}px (min: {LayoutConfig.MIN_ROOM_SIZE_LIMIT}px)"
            return [], [], [], [], []
        # prüfung raumanzahl
        if num_rooms < 1:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = "min 1 raum"
            return [], [], [], [], []
        # max räume
        max_rooms = int((self.max_size[0] * self.max_size[1]) / (min_room_size * min_room_size))
        if num_rooms > max_rooms:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = f"{num_rooms} räume passen nicht (max: {max_rooms})"
            return [], [], [], [], []
        # prüfung exits/entrances
        if num_exits < 1 or num_entrance < 0:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = "ungültige exit/entrance anzahl"
            return [], [], [], [], []
        # prüfung türbreite
        if door_width[0] < LayoutConfig.MIN_DOOR_WIDTH:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = f"tür zu schmal: {door_width[0]}px (min: {LayoutConfig.MIN_DOOR_WIDTH}px)"
            return [], [], [], [], []

        RoomLayout.last_error_reason = ""

        # teile arbeitsbereich in räume
        max_size = self.max_size
        self.num_rooms = num_rooms
        self.num_exits = num_exits
        self.num_entrance = num_entrance
        self.num_obstacles = num_obstacles
        self.min_room_size = min_room_size
        self.obstacle_size = obstacle_size
        self.door_width = door_width

        room_areas = [box(0, 0, max_size[0], max_size[1])]
        splitt_areas, rooms, hallways, neighbours, connected_rooms = [], [],  [], [], []
        obstacle_list, exits, entrance = [], [], []

        # split rooms horizontal oder vertikal
        for i in range(num_rooms - 1):
            room = None
            hor_ver = None
            coords = []
            tried_rooms = set()
            all_rooms_tried = False
            
            while room is None and not all_rooms_tried:
                available_rooms = [r for r in room_areas if r not in tried_rooms]
                if not available_rooms:
                    pass
                    all_rooms_tried = True
                    break
                    
                room = random.choice(available_rooms)
                tried_rooms.add(room)
                
                coords = room.exterior.xy
                
                can_split_horizontally = True
                can_split_vertically = True
                
                if ((max(coords[1]) - min(coords[1])) <= 2*min_room_size >= (max(coords[0]) - min(coords[0])) or
                    min(coords[1]) + min_room_size >= max(coords[1]) - min_room_size):
                    can_split_horizontally = False
                
                if ((max(coords[0]) - min(coords[0])) <= 2*min_room_size >= (max(coords[1]) - min(coords[1])) or
                    min(coords[0]) + min_room_size >= max(coords[0]) - min_room_size):
                    can_split_vertically = False
                
                if can_split_horizontally and can_split_vertically:
                    hor_ver = np.random.choice(['horizontal', 'vertical'])
                elif can_split_horizontally:
                    hor_ver = 'horizontal'
                elif can_split_vertically:
                    hor_ver = 'vertical'
                else:
                    room = None
            
            if room is None or all_rooms_tried:
                RoomLayout.layouts_rejected += 1
                RoomLayout.last_error_reason = "raumaufteilung fehlgeschlagen"
                return [], [], [], [], []

            if hor_ver == 'horizontal':
                x1 = (0, self.max_size[0])
                y1 = random.randint(int(min(coords[1]) + min_room_size), int(max(coords[1]) - min_room_size))
                splitt_areas = ops.split(room, LineString([(x1[0], y1), (x1[1], y1)]))
            elif hor_ver == 'vertical':
                x1 = random.randint(int(min(coords[0]) + min_room_size), int(max(coords[0]) - min_room_size))
                y1 = (0, self.max_size[1])
                splitt_areas = ops.split(room, LineString([(x1, y1[0]), (x1, y1[1])]))
            room_areas.remove(room)
            room_areas.append(get_geometry(splitt_areas, 0))
            room_areas.append(get_geometry(splitt_areas, 1))

        # räume erstellen
        minx, miny, maxx, maxy = self.max_size[0], self.max_size[1], 0, 0
        for area in room_areas:
            minx1, miny1, maxx1, maxy1 = area.bounds
            x1 = random.randint(int(minx1), int(minx1 + ((maxx1 - minx1) // LayoutConfig.ROOM_POSITION_VARIANCE_X)))
            y1 = random.randint(int(miny1), int(miny1 + ((maxy1 - miny1) // LayoutConfig.ROOM_POSITION_VARIANCE_Y)))
            border_dist = random.randint(LayoutConfig.BORDER_DISTANCE_RANGE[0], LayoutConfig.BORDER_DISTANCE_RANGE[1])
            rooms.append(box(x1+border_dist, y1+border_dist, maxx1 - border_dist, maxy1 - border_dist))
            if x1+border_dist < minx:
                minx = x1+border_dist
            if y1+border_dist < miny:
                miny = y1+border_dist
            if maxx1-border_dist > maxx:
                maxx = maxx1-border_dist
            if maxy1-border_dist > maxy:
                maxy = maxy1-border_dist

            self.room_shape.append(min((maxx1-border_dist)-(x1+border_dist), (maxy1-border_dist)-(y1+border_dist)) /
                                   max((maxx1-border_dist)-(x1+border_dist), (maxy1-border_dist)-(y1+border_dist)))
            self.room_area.append(((maxx1-border_dist)-(x1+border_dist))*((maxy1-border_dist)-(y1+border_dist)))
        self.building_area = (maxx - minx) * (maxy - miny)
        self.building_area_to_room_area_ratio = sum(self.room_area) / self.building_area
        if print_debug:
            gpd.GeoSeries([*rooms]).boundary.plot()
            plt.xlim([0, self.max_size[0]])
            plt.ylim([0, self.max_size[1]])
            plt.show()

        # korridore zwischen räumen
        neighbors = {r: set() for r in rooms}
        door_width = random.randint(door_width[0], door_width[1])
        for r1 in rooms:
            minx1, miny1, maxx1, maxy1 = [int(x) for x in r1.bounds]
            for r2 in rooms:
                if r1 == r2:
                    continue
                minx2, miny2, maxx2, maxy2 = [int(x) for x in r2.bounds]
                if (max(minx1, minx2) < min(maxx1, maxx2) - door_width
                        or max(miny1, miny2) < min(maxy1, maxy2) - door_width):
                    neighbors[r1].add((r2, r1.distance(r2)))

        for r1, value in neighbors.items():
            minx1, miny1, maxx1, maxy1 = r1.bounds
            shortest_path = None

            for r2 in value:
                if tuple((r2[0], r1)) in connected_rooms:
                    continue
                if shortest_path is None:
                    shortest_path = r2
                elif r2[1] < shortest_path[1]:
                    shortest_path = r2
            if shortest_path is not None and tuple((shortest_path[0], r1)) not in connected_rooms:
                connected_rooms.append((r1, shortest_path[0]))
                minx2, miny2, maxx2, maxy2 = shortest_path[0].bounds
                minx, miny, maxx, maxy = max(minx1, minx2), max(miny1, miny2), min(maxx1, maxx2), min(maxy1, maxy2)
                if max(minx1, minx2) < min(maxx1, maxx2):
                    x = random.randint(int(min(minx, maxx - door_width)),
                                       int(max(minx, maxx - door_width)))
                    hallways.append(box(x, int(miny), x + door_width, int(maxy)))

                    self.corridor_length_to_width_ratio.append(door_width / abs(int(maxy)-int(miny)))
                    self.room_width_to_door_width_ratio.append(door_width / (int(maxx1)-int(minx1)))
                    self.room_width_to_door_width_ratio.append(door_width / (int(maxx2)-int(minx2)))
                    self.door_orientation.append((int(maxx1)-int(minx1)) / (int(maxy1)-int(miny1)))
                    self.door_orientation.append((int(maxx2)-int(minx2)) / (int(maxy2)-int(miny2)))
                    self.door_position.append(min((x-int(minx1)), (int(maxx1)-x-door_width)) /
                                              max((x-int(minx1)), (int(maxx1)-x-door_width)))
                    self.door_position.append(min((x - int(minx2)), (int(maxx2) - x - door_width)) /
                                              max((x - int(minx2)), (int(maxx2) - x - door_width)))
                    self.hallway_area.append(door_width*abs(int(maxy)-int(miny)))
                elif max(miny1, miny2) < min(maxy1, maxy2):
                    y = random.randint(int(min(miny, maxy - door_width)),
                                       int(max(miny, maxy - door_width)))
                    hallways.append(box(int(minx), y, int(maxx), y + door_width))

                    self.corridor_length_to_width_ratio.append(door_width / abs(int(maxx)-int(minx)))
                    self.room_width_to_door_width_ratio.append(door_width / (int(maxy1) - int(miny1)))
                    self.room_width_to_door_width_ratio.append(door_width / (int(maxy2) - int(miny2)))
                    self.door_orientation.append((int(maxy1) - int(miny1)) / (int(maxx1) - int(minx1)))
                    self.door_orientation.append((int(maxy2) - int(miny2)) / (int(maxx2) - int(minx2)))
                    self.door_position.append(min((y - int(miny1)), (int(maxy1) - y - door_width)) /
                                              max((y - int(miny1)), (int(maxy1) - y - door_width)))
                    self.door_position.append(min((y - int(miny2)), (int(maxy2) - y - door_width)) /
                                              max((y - int(miny2)), (int(maxy2) - y - door_width)))
                    self.hallway_area.append(door_width * abs(int(maxx) - int(minx)))

        if print_debug:
            gpd.GeoSeries([*hallways, *rooms]).boundary.plot()
            plt.xlim([0, self.max_size[0]])
            plt.ylim([0, self.max_size[1]])
            plt.show()

        # exits einfügen
        multi_poly = union_all(hallways + rooms)
        from shapely.geometry import MultiPolygon, Polygon
        
        if isinstance(multi_poly, MultiPolygon):
            # größtes polygon
            largest_poly = sorted(multi_poly.geoms, key=lambda p: p.area, reverse=True)[0]
            ext_multi = largest_poly.exterior
        else:
            ext_multi = multi_poly.exterior
        tried_wall_segments = set()
        max_attempts = len(ext_multi.xy[0]) - 1
        attempts = 0
        all_segments_tried = False
        
        while len(exits) != num_exits and not all_segments_tried:
            available_segments = [i for i in range(len(ext_multi.xy[0])-1) if i not in tried_wall_segments]
            
            if not available_segments:
                pass
                all_segments_tried = True
                break
            
            i_ = random.choice(available_segments)
            tried_wall_segments.add(i_)
            attempts += 1
            
            x1, y1 = ext_multi.xy[0][i_], ext_multi.xy[1][i_]
            x2, y2 = ext_multi.xy[0][i_ + 1], ext_multi.xy[1][i_ + 1]
            
            if x1 == x2 and min(y1, y2) < max(y1, y2)-door_width:
                # vertikal
                y = random.randint(int(min(y1, y2)), int(max(y1, y2)-door_width))
                exits.append(LineString([(x1, y), (x2, y + door_width)]))
                self.exit_to_room_ratio.append(door_width / abs(int(y2) - int(y1)))
                self.exit_wall_points[(x1, y), (x2, y + door_width)] = [(int(x1), int(y1)), (int(x2), int(y2))]
            elif y1 == y2 and min(x1, x2) < max(x1, x2)-door_width:
                # horizontal
                x = random.randint(int(min(x1, x2)), int(max(x1, x2)-door_width))
                exits.append(LineString([(x, y1), (x + door_width, y2)]))
                self.exit_to_room_ratio.append(door_width / abs(int(x2) - int(x1)))
                self.exit_wall_points[(x, y1), (x + door_width, y2)] = [(int(x1), int(y1)), (int(x2), int(y2))]
        
        # prüfe ob alle exits platziert wurden
        if len(exits) < num_exits:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = f"nur {len(exits)} von {num_exits} exits platzierbar"
            return [], [], [], [], []

        # entrances platzieren
        tried_entrance_segments = set()
        all_entrance_segments_tried = False
        
        while len(entrance) != num_entrance and not all_entrance_segments_tried:
            available_entrance_segments = [i for i in range(len(ext_multi.xy[0])-1) if i not in tried_entrance_segments]
            
            if not available_entrance_segments:
                pass
                all_entrance_segments_tried = True
                break
            
            i_ = random.choice(available_entrance_segments)
            tried_entrance_segments.add(i_)
            
            x1, y1 = ext_multi.xy[0][i_], ext_multi.xy[1][i_]
            x2, y2 = ext_multi.xy[0][i_ + 1], ext_multi.xy[1][i_ + 1]
            
            if x1 == x2 and min(y1, y2) + door_width < max(y1, y2) - door_width:
                # vertikal
                y = random.randint(int(min(y1, y2) + door_width), int(max(y1, y2) - door_width))
                entrance.append(LineString([(x1, y), (x2, y + door_width)]))
            elif y1 == y2 and min(x1, x2) + door_width < max(x1, x2) - door_width:
                # horizontal
                x = random.randint(int(min(x1, x2) + door_width), int(max(x1, x2) - door_width))
                entrance.append(LineString([(x, y1), (x + door_width, y2)]))
        
        # prüfe ob alle entrances platziert wurden
        if len(entrance) < num_entrance:
            RoomLayout.layouts_rejected += 1
            RoomLayout.last_error_reason = f"nur {len(entrance)} von {num_entrance} entrances platzierbar"
            return [], [], [], [], []

        # hindernisplatzierung
        # 1) filtere zu kleine räume, 2) jedes hindernis in zufälligem raum versuchen
        # 3) brute-force mit max_attempts, 4) abstände und kollisionen prüfen
        
        if num_obstacles > 0:
            min_distance = LayoutConfig.MIN_WALL_DISTANCE  # abstand zu wänden
            # max versuche pro raum (bei overlap=false wichtig)
            max_attempts_per_room = LayoutConfig.MAX_OBSTACLE_ATTEMPTS
            obstacles_placed = 0
            total_attempts = 0

            # filtere zu kleine räume
            suitable_rooms = []
            for room_index, room in enumerate(rooms.copy()):
                room_minx, room_miny, room_maxx, room_maxy = room.bounds
                room_width = room_maxx - room_minx
                room_height = room_maxy - room_miny
                
                if room_width >= obstacle_size[0] + 2*min_distance and room_height >= obstacle_size[0] + 2*min_distance:
                    suitable_rooms.append(room)
                else:
                    pass  # zu klein

            if not suitable_rooms:
                RoomLayout.layouts_rejected += 1
                RoomLayout.last_error_reason = "kein raum groß genug"
                return [], [], [], [], []

            random.shuffle(suitable_rooms)  # für variation

            for obstacle_index in range(num_obstacles):
                obstacle_placed = False
                # alle räume durchprobieren
                for room_index, room in enumerate(suitable_rooms):
                    room_minx, room_miny, room_maxx, room_maxy = room.bounds
                    room_attempts = 0
                    room_width = room_maxx - room_minx
                    room_height = room_maxy - room_miny
                    # brute-force platzierung
                    max_possible_width = 0
                    max_possible_height = 0
                    while room_attempts < max_attempts_per_room and not obstacle_placed:
                        room_attempts += 1
                        total_attempts += 1
                        # sichere grenzen berechnen
                        max_x = int(room_maxx - obstacle_size[0] - min_distance)
                        max_y = int(room_maxy - obstacle_size[0] - min_distance)
                        # zufällige position
                        x = random.randint(int(room_minx + min_distance), max_x)
                        y = random.randint(int(room_miny + min_distance), max_y)
                        # max größe hier
                        max_possible_height = int(room_maxy - y - min_distance)
                        max_possible_width = int(room_maxx - x - min_distance)
                        height = random.randint(obstacle_size[0], min(obstacle_size[1], max_possible_height))
                        width = random.randint(obstacle_size[0], min(obstacle_size[1], max_possible_width))
                        obstacle = box(x, y, x+width, y+height)
                        # prüfe ob hindernis passt
                        if obstacle.within(room) and obstacle.distance(room.boundary) > min_distance:
                            place_obstacle = True
                            # kollisionen prüfen wenn nötig
                            if not allow_overlap:
                                for existing in obstacle_list:
                                    if obstacle.intersects(existing) or obstacle.distance(existing) < min_distance:
                                        place_obstacle = False
                                        break
                            
                            if place_obstacle:
                                obstacle_list.append(obstacle)
                                self.obstacle_area.append(height*width)
                                self.room_area_to_obstacle_area_ratio = sum(self.obstacle_area) / sum(self.room_area)
                                obstacle_placed = True
                                obstacles_placed += 1
                                break
                    if obstacle_placed:
                        # raum nach hinten für rotation
                        suitable_rooms.append(suitable_rooms.pop(room_index))
                        break
                    else:
                        if room_attempts >= max_attempts_per_room:
                            pass  # nicht platzierbar
                if not obstacle_placed:
                    break 
            # prüfe ob alle hindernisse platziert
            if obstacles_placed < num_obstacles:
                RoomLayout.layouts_rejected += 1
                if not allow_overlap:
                    RoomLayout.last_error_reason = f"nur {obstacles_placed} von {num_obstacles} hindernisse (nach {LayoutConfig.MAX_OBSTACLE_ATTEMPTS} versuchen pro raum)"
                else:
                    RoomLayout.last_error_reason = f"nur {obstacles_placed} von {num_obstacles} hindernisse"
                return [], [], [], [], []
        if len(obstacle_list) > 0:
            obstacle_geometries = union_all(obstacle_list)
            obstacle_geometries_for_coords = [obstacle_geometries]
        else:
            obstacle_geometries_for_coords = []
            obstacle_geometries = None
            

        self.exits = self._get_coords(exits)
        self.domainpolygon = self._get_coords([multi_poly])
        self.obstacle_geometries = self._get_coords(obstacle_geometries_for_coords)
        self.entrance = self._get_coords(entrance)
        
        self.rooms_geometry = rooms
        self.hallways_geometry = hallways
        self.obstacles_geometry = obstacle_list
        
        return self.domainpolygon, self.obstacle_geometries, self.exits, self.entrance, self.exit_wall_points

    @staticmethod
    def _get_coords(geometry):
        coords = []
        for geo in geometry:
            type_id = get_type_id(geo)
            if type_id == 6:
                for g in geo.geoms:
                    coord = g.exterior.xy
                    l1, l2 = coord[0].tolist(), coord[1].tolist()
                    coords.append(list(zip(l1, l2)))
            elif type_id == 7:
                return []
            else:
                if -1 <= type_id <= 1:
                    coord = geo.coords.xy
                else:
                    coord = geo.exterior.xy
                l1, l2 = coord[0].tolist(), coord[1].tolist()
                coords.append(list(zip(l1, l2)))
        return coords

    def generate_id(self, size, chars=string.ascii_letters + string.digits + '-' + '_'):
        return ''.join(random.choice(chars) for _ in range(size))

    def create_json(self, path="../data/generated.json", rooms=None, hallways=None, obstacle_list=None):
        # domain
        domain_poly = self.domainpolygon
        obstacle_geometries = self.obstacle_geometries
        segment_points = []
        point_order = []
        i = 0
        for poly in (domain_poly + obstacle_geometries):
            segment_points.append(int(poly[0][0]))
            segment_points.append(int(poly[0][1]))
            i_start = i
            point_order.append(i_start)
            for tuple_ in poly[1:-1]:
                i = i + 1
                segment_points.append(int(tuple_[0]))
                segment_points.append(int(tuple_[1]))
                point_order.append(i)
                point_order.append(i)
            point_order.append(i_start)
            i = i + 1
        
        num_points = len(segment_points) // 2
        num_segments = num_points
        
        holes = []
        for obstacle in obstacle_geometries:
            x = [w[0] for w in obstacle]
            y = [w[1] for w in obstacle]
            holes.append((sum(x) / len(x), sum(y) / len(y)))

        polygon_corners = []
        for polygon in domain_poly:
            corner = []
            for p in polygon:
                corner.append((int(p[0]), int(p[1]), self.generate_id(21)))
            polygon_corners.append(corner)

        exit_ids = {}
        for e in self.exits:
            wall = self.exit_wall_points[(e[0], e[1])]
            id_ = ''
            for w in wall:
                for polygon_corner in polygon_corners:
                    for p in polygon_corner:
                        if p[0] == w[0] and p[1] == w[1]:
                            id_ = id_ + p[2]
                            break
            exit_ids[(e[0], e[1])] = id_

        hole_corners = []
        for obstacle in obstacle_geometries:
            corner = []
            for w in obstacle:
                corner.append((int(w[0]), int(w[1]), self.generate_id(21)))
            hole_corners.append(corner[:-1])

        config = dict(
            name=self.name,
            Scenario={
                "charLength": self.char_length,
                "charDensity": self.char_density,
                "charVelocity": self.char_velocity,
                "startPersons": self.start_persons,
                "randPersons": self.rand_persons,
                "maxTime": self.max_time
            },
            Fundamentaldiagramm=self.fd,
            Infection={
                "percentInfected": self.percent_infected,
                "criticalDistance": self.critical_distance,
                "infectionRate": self.infection_rate,
                "percentRemoved": self.percent_removed,
                "resistanceTime": self.resistance_time,
                "moveMode": self.move_mode
            },
            Measurementstations=[],
            Entrance=[
                {
                    "wallId": self.generate_id(42),
                    "xr": e[0][0],
                    "yr": e[0][1],
                    "xl": e[1][0],
                    "yl": e[1][1],
                    "personPerSecond": 0,
                    "maxPersons": 0
                } for e in self.entrance
            ],
            Exit=[
                {
                    "wallId": exit_ids[(e[0], e[1])],
                    "xr": int(e[0][0]),
                    "yr": int(e[0][1]),
                    "xl": int(e[1][0]),
                    "yl": int(e[1][1]),
                    "weight": 1
                } for e in self.exits
            ],
            Attractors=self.attractors,
            SubdomainsFD=self.subdomains_fd,
            SubdomainsRhoInit=self.subdomains_rho_init,
            Refinement={
                "decompositionLevel": self.decomposition_level,
                "localRefinement": self.local_refinement,
                "localMarkMethod": self.local_mark_method,
                "globalRefinement": self.global_refinement
            },
            Domainpolygon=dict(
                numberPoints=num_points,
                segmentPoints=segment_points,
                numberSegments=num_segments,
                pointOrder=point_order,
                numberHoles=len(obstacle_geometries),
                holes=[
                    {
                        "x": x,
                        "y": y
                    } for (x, y) in holes
                ]),
            Grid={},
            PolygonCorners={
                "corners": [
                    {
                        "id": c[2],
                        "x": c[0],
                        "y": c[1]
                    } for c in polygon_corners[0]
                ],
                "closed": True
            },
            HoleCorners=[
                {
                    "corners": [
                        {
                            "id": c[2],
                            "xr": c[0],
                            "y": c[1]
                        } for c in hole_corner
                    ],
                    "closed": True
                } for hole_corner in hole_corners
            ])

        config["Analysis"] = {
            "room_width_to_door_width_ratio": self.room_width_to_door_width_ratio,
            "door_orientation": self.door_orientation,
            "room_shape": self.room_shape,
            "door_position": self.door_position,
            "corridor_length_to_width_ratio": self.corridor_length_to_width_ratio,
            "building_area_to_room_area_ratio": self.building_area_to_room_area_ratio,
            "room_area_to_obstacle_area_ratio": self.room_area_to_obstacle_area_ratio,
            "exit_to_room_ratio": self.exit_to_room_ratio,
            "room_area": self.room_area,
            "hallway_area": self.hallway_area,
            "building_area": self.building_area,
            "obstacle_area": self.obstacle_area,
            "num_rooms": self.num_rooms,
            "num_exits": self.num_exits,
            "num_entrance": self.num_entrance,
            "num_obstacles": self.num_obstacles,
            "min_room_size": self.min_room_size,
            "obstacle_size": self.obstacle_size,
            "door_width": self.door_width
        }
        
        # visualisierungsdaten
        if hasattr(self, 'rooms_geometry') and self.rooms_geometry:
            config["Visualization"] = {
                "rooms": [
                    {
                        "id": i,
                        "bounds": list(room.bounds),
                        "coords": [(x, y) for x, y in zip(*room.exterior.xy)]
                    } for i, room in enumerate(self.rooms_geometry)
                ],
                "hallways": [
                    {
                        "id": i,
                        "bounds": list(hallway.bounds),
                        "coords": [(x, y) for x, y in zip(*hallway.exterior.xy)]
                    } for i, hallway in enumerate(self.hallways_geometry)
                ]
            }
        json_file = json.dumps(config, indent=2)

        with open(path, "w") as jsonfile:
            jsonfile.write(json_file)
        return json_file


def main():
    layout = RoomLayout()
    layout.create_polygon_layout()
    layout.print_analysis()
    layout.create_json()


if __name__ == "__main__":
    main()
