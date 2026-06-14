import os
import json
import numpy as np
from scipy.ndimage import distance_transform_edt
from skimage.segmentation import flood_fill


def scale_points(segment_points, exit_points, target_size):
    max_points = max(segment_points) + 1
    scale_factor = target_size / max_points

    for idx, point in enumerate(segment_points):
        segment_points[idx] = int(point * scale_factor)

    exit_points_list = []
    for p in exit_points:
        p['xr'] = int(p['xr'] * scale_factor)
        p['yr'] = int(p['yr'] * scale_factor)
        p['xl'] = int(p['xl'] * scale_factor)
        p['yl'] = int(p['yl'] * scale_factor)
        exit_points_list.append([p['xr'], p['yr'], p['xl'], p['yl']])

    return segment_points, exit_points_list


def get_geometry(data, target_size=(256, 256)):
    """
    Generates a cropped bitmap of the given data, along with the exit points.
    Konfiguration der Flow region (0=default, 1=Wall + obstacle, 3=Entrance, 4=Exit, 5=Persons_density,
                                   6=empty region)
    :param target_size:
    :param data: JSON data as a dictionary. Domainpolygon containing segmentPoints and pointOrder.
                 A list of exit points.
    :return cropped_bitmap: A cropped bitmap as a 2D array.
    :return exit_points: List of exit points.
    """
    # Validiere Eingabedaten
    if 'Domainpolygon' not in data:
        raise ValueError("JSON-Daten enthalten kein 'Domainpolygon'")
    if 'segmentPoints' not in data['Domainpolygon']:
        raise ValueError("Domainpolygon enthält keine 'segmentPoints'")
    if 'pointOrder' not in data['Domainpolygon']:
        raise ValueError("Domainpolygon enthält keine 'pointOrder'")
    if 'Exit' not in data:
        raise ValueError("JSON-Daten enthalten keine 'Exit' Punkte")
    segment_points = data['Domainpolygon']['segmentPoints']
    point_order = data['Domainpolygon']['pointOrder']
    exit_points = data['Exit']

    # hier skalieren, da sonst das ergebniss nicht genau genug wird
    segment_points, exit_points = scale_points(segment_points, exit_points, target_size[0])

    max_points = max(segment_points) + 1

    array = np.zeros((max_points, max_points), dtype=int)

    line_segments = []
    zip_list = list(zip(point_order, segment_points))
    element = []
    for p in range(len(segment_points)):
        try:
            if zip_list[p][0] + 1 == zip_list[p + 1][0] or zip_list[p][0] == zip_list[p + 1][0]:
                element.append(zip_list[p])
            if zip_list[p][0] > zip_list[p + 1][0]:
                element.append(zip_list[p])
                element.append(zip_list[p + 1])
                line_segments.append(element)
                element = []
            else:
                continue
        except IndexError:
            continue

    for idx, e in enumerate(line_segments):
        if idx == 0:
            color = 1
        else:
            color = 2
        for i in range(0, len(e), 2):
            try:  # Anfangs- und Endpunkt der Linie
                start = (e[i], e[i + 1])
                end = (e[i + 2], e[i + 3])
            except IndexError:
                start = (e[i], e[i + 1])
                end = (e[0], e[1])

            num_points = int((abs(start[0][1] - start[1][1]) + abs(end[0][1] - end[1][1])))
            coordinates = list(zip(np.linspace(start[0][1], end[0][1], num_points*2),
                                   np.linspace(start[1][1], end[1][1], num_points*2)))
            array[[int(coord[0]) for coord in coordinates], [int(coord[1]) for coord in coordinates]] = color
        if idx == 0:
            array = fill_segment(array, line_seg=e, color=6)
            #array = fill_segment(array, color=1, start_point=(0, 0))
        else:
            array = fill_segment(array, line_seg=e, color=2)

    exits = []
    for ep in exit_points:
        start = (ep[0], ep[1])
        end = (abs(ep[2]), abs(ep[3]))
        exits.append((start, end))
        num_points = int((abs(start[0] - start[1]) + abs(end[0] - end[1])))
        coordinates = list(zip(np.linspace(start[0], end[0], num_points),
                               np.linspace(start[1], end[1], num_points)))
        array[[int(coord[0]) for coord in coordinates], [int(coord[1]) for coord in coordinates]] = 4

    array = np.rot90(np.flip(array, axis=0), k=3)
    sdf_array = sdf(array, [(2, -1), (1, -1), (4, -10), (6, 1)])

    return np.array([array, sdf_array], dtype=int).tolist(), exit_points


def fill_segment(array, color, line_seg=[], start_point=None):
    if len(line_seg) > 1:
        min_x_tuple = min(line_seg[0::2], key=lambda x: x[1])
        min_x_tuples = [(x, idx*2) for idx, x in enumerate(line_seg[0::2]) if x[1] == min_x_tuple[1]]
        min_y_tuples = [line_seg[x[1] + 1] for x in min_x_tuples]
        min_y_tuple = min(min_y_tuples, key=lambda x: x[1])

        start_point = (min_x_tuple[1]+1, min_y_tuple[1]+1)

    array = flood_fill(array, start_point, color)
    return array


def sdf(segment, values):
    """Berechnet Signed Distance Field für verschiedene Segment-Werte."""
    segment_result = np.zeros_like(segment, dtype=np.float32)
    for value in values:
        sdf_result = distance_transform_edt(segment == value[0])
        sdf_result[segment == value[0]] *= value[1]
        coords = np.argwhere(segment == value[0])
        for x, y in coords:
            segment_result[x, y] = sdf_result[x][y]
    return segment_result


def read_json(json_path, size_geometry=(256, 256)):
    """
    Read the JSON file and extract the necessary information.
    :param size_geometry:
    :param json_path: The path to the JSON file.
    :return: A tuple containing the following values:
            - dens (float): Characteristic density.
            - vel (float): Characteristic velocity.
            - start_pers (int): Number of persons at the start of the simulation.
            - fd (list): Fundamental diagram.
            - exit_points (list): Exit points.
            - geometry_im (list): Geometry image.
            - analysis_param (dict): Analysis parameters.
    """
    if not os.path.exists(json_path):
        raise FileNotFoundError(f"JSON-Datei nicht gefunden: {json_path}")
    
    try:
        with open(json_path, "r") as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        raise ValueError(f"Ungültige JSON-Datei: {json_path}. Fehler: {str(e)}")
    
    # Validiere notwendige Felder
    required_fields = {
        'Scenario': ['charDensity', 'charVelocity', 'startPersons'],
        'Fundamentaldiagramm': [],
        'Analysis': []
    }
    
    for field, subfields in required_fields.items():
        if field not in data:
            raise ValueError(f"JSON-Datei enthält kein '{field}' Feld")
        for subfield in subfields:
            if subfield not in data[field]:
                raise ValueError(f"'{field}' enthält kein '{subfield}' Feld")
    
    dens = data['Scenario']['charDensity']
    vel = data['Scenario']['charVelocity']
    start_pers = data['Scenario']['startPersons']
    fd = data['Fundamentaldiagramm']
    geometry_im, exit_points = get_geometry(data, target_size=size_geometry)
    analysis_param = data['Analysis']
    
    return dens, vel, start_pers, fd, exit_points, geometry_im, analysis_param
