# layout visualisierung

import os
import sys
import glob
import json
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as MPLPolygon
from config import TechnicalConstants


def visualize_layout(json_path, save_path=None, silent=False):
    with open(json_path, 'r') as f:
        data = json.load(f)
    
    fig, ax = plt.subplots(1, 1, figsize=(12, 12))
    ax.set_facecolor('white')
    
    # domain polygon
    points = data['Domainpolygon']['segmentPoints']
    point_order = data['Domainpolygon']['pointOrder']
    
    # koordinaten mit y-transform
    coords = [(points[i*2], TechnicalConstants.Y_TRANSFORM_OFFSET - points[i*2+1]) for i in range(len(points)//2)]
    
    # polygone aus point_order
    polygons = []
    current_polygon = []
    
    i = 0
    while i < len(point_order):
        if i > 0 and (i + 1 < len(point_order)) and point_order[i] == point_order[i+1]:
            current_polygon.append(coords[point_order[i]])
            i += 2
        else:
            if current_polygon:
                current_polygon.append(coords[point_order[i]])
                polygons.append(current_polygon)
                current_polygon = []
            else:
                current_polygon.append(coords[point_order[i]])
            i += 1
    
    if current_polygon:
        polygons.append(current_polygon)
    
    if 'Visualization' in data:
        # räume
        colors = ['lightcoral', 'lightgreen', 'lightsalmon', 'lightsteelblue', 
                 'plum', 'khaki', 'lightpink', 'lightcyan']
        for room in data['Visualization']['rooms']:
            room_coords = [(x, TechnicalConstants.Y_TRANSFORM_OFFSET - y) for x, y in room['coords']]
            room_poly = MPLPolygon(room_coords[:-1], fill=True, 
                                  facecolor=colors[room['id'] % len(colors)], 
                                  edgecolor='black', linewidth=2, alpha=0.6)
            ax.add_patch(room_poly)
            bounds = room['bounds']
            center_x = (bounds[0] + bounds[2]) / 2
            center_y = (TechnicalConstants.Y_TRANSFORM_OFFSET - bounds[1] + TechnicalConstants.Y_TRANSFORM_OFFSET - bounds[3]) / 2
            ax.text(center_x, center_y, f"R{room['id']+1}", 
                   ha='center', va='center', fontsize=12, fontweight='bold')
        
        # korridore
        for hallway in data['Visualization']['hallways']:
            hallway_coords = [(x, TechnicalConstants.Y_TRANSFORM_OFFSET - y) for x, y in hallway['coords']]
            hallway_poly = MPLPolygon(hallway_coords[:-1], fill=True,
                                     facecolor='yellow', edgecolor='orange',
                                     linewidth=2, alpha=0.8)
            ax.add_patch(hallway_poly)
    else:
        # fallback
        if polygons:
            domain_poly = MPLPolygon(polygons[0], fill=True, facecolor='lightblue', 
                                    edgecolor='black', linewidth=3, alpha=0.3)
            ax.add_patch(domain_poly)
    
    # hindernisse
    for hole in data.get('HoleCorners', []):
        hole_coords = [(c.get('xr', c.get('x', 0)), TechnicalConstants.Y_TRANSFORM_OFFSET - c.get('y', 0)) for c in hole['corners']]
        if hole_coords and len(hole_coords) >= 3:
            hole_poly = MPLPolygon(hole_coords, fill=True, facecolor='white',
                                  edgecolor='red', linewidth=2)
            ax.add_patch(hole_poly)
    
    # exits
    for exit in data['Exit']:
        x1, y1 = exit['xr'], TechnicalConstants.Y_TRANSFORM_OFFSET - exit['yr']
        x2, y2 = exit['xl'], TechnicalConstants.Y_TRANSFORM_OFFSET - exit['yl']
        ax.plot([x1, x2], [y1, y2], 'g-', linewidth=4, label='Exit' if exit == data['Exit'][0] else '')
    
    # entrances
    for entrance in data['Entrance']:
        x1, y1 = entrance['xr'], TechnicalConstants.Y_TRANSFORM_OFFSET - entrance['yr']
        x2, y2 = entrance['xl'], TechnicalConstants.Y_TRANSFORM_OFFSET - entrance['yl']
        ax.plot([x1, x2], [y1, y2], 'b-', linewidth=4, label='Entrance' if entrance == data['Entrance'][0] else '')
    
    if 'Analysis' in data:
        analysis = data['Analysis']
        title = (f"Layout: {data['name']}\n"
                f"Räume: {analysis.get('num_rooms', 'N/A')}, "
                f"Ausgänge: {analysis.get('num_exits', 'N/A')}, "
                f"Hindernisse: {analysis.get('num_obstacles', 'N/A')}")
    else:
        title = f"Layout: {data['name']}"
    
    ax.set_xlim(0, TechnicalConstants.Y_TRANSFORM_OFFSET)
    ax.set_ylim(0, TechnicalConstants.Y_TRANSFORM_OFFSET)
    ax.set_aspect('equal')
    ax.set_title(title, fontsize=14)
    ax.legend(loc='upper right')
    ax.grid(True, alpha=0.3)
    
    if save_path:
        plt.savefig(save_path, dpi=150, bbox_inches='tight')
        if not silent:
            print(f"saved: {save_path}")
    else:
        plt.show()
    
    plt.close()


def visualize_all_layouts(directory_path, max_layouts=20):
    json_files = glob.glob(os.path.join(directory_path, "*.json"))[:max_layouts]
    
    if not json_files:
        print(f"keine json in {directory_path}")
        return
    
    print(f"visualisiere {len(json_files)} layouts")
    
    img_dir = os.path.join(directory_path, "layout_images")
    os.makedirs(img_dir, exist_ok=True)
    
    for i, json_file in enumerate(json_files):
        img_path = os.path.join(img_dir, f"{os.path.basename(json_file)[:-5]}.png")
        try:
            visualize_layout(json_file, save_path=img_path, silent=True)
        except Exception as e:
            print(f"  fehler: {e}")
    
    print(f"bilder in: {img_dir}")


def main():
    if len(sys.argv) == 1:
        print("layout visualizer")
        path = input("pfad zu json oder verzeichnis: ").strip()
    else:
        path = sys.argv[1]
    
    if not os.path.exists(path):
        print(f"fehler: '{path}' existiert nicht")
        return
    
    if os.path.isdir(path):
        max_layouts = 20
        if len(sys.argv) > 2:
            try:
                max_layouts = int(sys.argv[2])
            except:
                pass
        visualize_all_layouts(path, max_layouts)
    elif path.endswith('.json'):
        print(f"visualisiere: {path}")
        parent_dir = os.path.dirname(path) or '.'
        img_dir = os.path.join(parent_dir, "layout_images")
        os.makedirs(img_dir, exist_ok=True)
        img_path = os.path.join(img_dir, f"{os.path.basename(path)[:-5]}.png")
        visualize_layout(path, save_path=img_path)
    else:
        print(f"fehler: '{path}' ist keine json")


if __name__ == "__main__":
    main()