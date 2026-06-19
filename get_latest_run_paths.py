#!/usr/bin/env python3
# pfade zum neuesten run finden

import os
import sys
from pathlib import Path
# generate zum path
#sys.path.append("home/dbohnet/PycharmProjects/ping/generate")
sys.path.append(r"C:\Users\doris\PycharmProjects\ping\generate")

from generate.split_tfrecords import get_split_paths

#----------------------
# Find folder with newest tfrecords files
#-----------------------

def get_latest_run_dir(project_root=None):

    if project_root is None:
        project_root = Path(__file__).parent
    
    runs_dir = Path(project_root) / "data" / "runs"
    
    if not runs_dir.exists():
        raise ValueError(f"runs directory is missing: {runs_dir}")
    
    run_dirs = [d for d in runs_dir.iterdir() if d.is_dir()]
    
    if not run_dirs:
        raise ValueError("no tfrecords data found")
    
    # choose the newest
    latest_run = max(run_dirs, key=lambda d: d.stat().st_ctime)
    
    return latest_run


#----------------------
# Set path to tfrecord files for training
#-----------------------

def get_tfrecord_paths_for_run(run_name=None, project_root=None):
    """path to tfrecord files for run"""
    if project_root is None:
        project_root = Path(__file__).parent

    if run_name:
        run_dir = Path(project_root) / "data" / "runs" / run_name
    else:
        run_dir = get_latest_run_dir(project_root)
    
    tfrecord_dir = run_dir / "tfrecords"
    
    if not tfrecord_dir.exists():
        raise ValueError(f"tfrecord directory is missing: {tfrecord_dir}")
    
    # split paths for training/validation and test data set
    split_paths = get_split_paths(str(tfrecord_dir))
    
    return str(tfrecord_dir), split_paths

#-----------------
# MAIN to test routine get_latest_run_paths
#------------------

def main():
    try:
        tfrecord_dir, paths = get_tfrecord_paths_for_run()
        
        print(f"newest run: {Path(tfrecord_dir).parent.name}")
        print(f"tfrecord dir: {tfrecord_dir}")
        print(f"train: {len(paths['train'])} files")
        print(f"val: {len(paths['validation'])} files")
        print(f"test: {len(paths['test'])} files")
            
    except ValueError as e:
        print(f"error: {e}")
        print("run DataGeneration.py first or check structure of project folder, no tfrecord files found")


if __name__ == "__main__":
    main()