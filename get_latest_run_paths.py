#!/usr/bin/env python3
# pfade zum neuesten run finden

import os
import sys
from pathlib import Path
# generate zum path
sys.path.append(str(Path(__file__).resolve().parent.parent))

from generate.split_tfrecords import get_split_paths


def get_latest_run_dir(project_root=None):
    """neuestes run verzeichnis"""
    if project_root is None:
        project_root = Path(__file__).parent
    
    runs_dir = Path(project_root) / "data" / "runs"
    
    if not runs_dir.exists():
        raise ValueError(f"runs dir fehlt: {runs_dir}")
    
    run_dirs = [d for d in runs_dir.iterdir() if d.is_dir()]
    
    if not run_dirs:
        raise ValueError("keine runs gefunden")
    
    # nach zeit sortieren
    latest_run = max(run_dirs, key=lambda d: d.stat().st_ctime)
    
    return latest_run


def get_tfrecord_paths_for_run(run_name=None, project_root=None):
    """tfrecord pfade für run"""
    if project_root is None:
        project_root = Path(__file__).parent
    
    if run_name:
        run_dir = Path(project_root) / "data" / "runs" / run_name
    else:
        run_dir = get_latest_run_dir(project_root)
    
    tfrecord_dir = run_dir / "tfrecords"
    
    if not tfrecord_dir.exists():
        raise ValueError(f"tfrecord dir fehlt: {tfrecord_dir}")
    
    # split pfade
    split_paths = get_split_paths(str(tfrecord_dir))
    
    return str(tfrecord_dir), split_paths


def main():
    try:
        tfrecord_dir, paths = get_tfrecord_paths_for_run()
        
        print(f"neuester run: {Path(tfrecord_dir).parent.name}")
        print(f"tfrecord dir: {tfrecord_dir}")
        print(f"train: {len(paths['train'])} dateien")
        print(f"val: {len(paths['validation'])} dateien")
        print(f"test: {len(paths['test'])} dateien")
            
    except ValueError as e:
        print(f"fehler: {e}")
        print("erst DataGeneration.py laufen lassen")


if __name__ == "__main__":
    main()