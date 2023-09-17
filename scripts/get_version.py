#!/usr/bin/python3

'''Get version from version.json'''

import json
import sys
from pathlib import Path


def get_version(version_type):
    '''Get version from version.json'''

    top_directory = Path(__file__).resolve(True).parents[1]
    with top_directory.joinpath('version.json').open(encoding='utf-8') as version_file:
        version = json.load(version_file)
        print(version[version_type])


if __name__ == "__main__":
    get_version(sys.argv[1] if len(sys.argv) > 1 else "package_version")
