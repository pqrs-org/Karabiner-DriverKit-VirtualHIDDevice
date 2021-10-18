#!/usr/bin/python3

import os
import re
import sys
from pathlib import Path
from itertools import chain

top_directory = Path(__file__).resolve(True).parents[1]

with top_directory.joinpath('version').open() as version_file, top_directory.joinpath('driver-version').open() as driver_version_file:
    version = version_file.readline().strip()
    version_number = 0
    for v in version.split('.'):
        version_number = version_number * 100 + int(v)

    driver_version = driver_version_file.readline().strip()
    driver_version_number = 0
    for v in driver_version.split('.'):
        driver_version_number = driver_version_number * 100 + int(v)

    for template_file_path in chain(top_directory.rglob('*.hpp.in'),
                                    top_directory.rglob('*.plist.in'),
                                    top_directory.rglob('*.xml.in')):
        replaced_file_path = Path(
            re.sub(r'\.in$', '', str(template_file_path)))
        needs_update = False

        with template_file_path.open('r') as template_file:
            template_lines = template_file.readlines()
            replaced_lines = []

            if replaced_file_path.exists():
                with replaced_file_path.open('r') as replaced_file:
                    replaced_lines = replaced_file.readlines()
                    while len(replaced_lines) < len(template_lines):
                        replaced_lines.append('')
            else:
                replaced_lines = template_lines

            for index, template_line in enumerate(template_lines):
                line = template_line
                line = line.replace('@VERSION@', version)
                line = line.replace('@VERSION_NUMBER@', str(version_number))
                line = line.replace('@DRIVER_VERSION@', driver_version)
                line = line.replace('@DRIVER_VERSION_NUMBER@',
                                    str(driver_version_number))

                if (replaced_lines[index] != line):
                    needs_update = True
                    replaced_lines[index] = line

        if needs_update:
            with replaced_file_path.open('w') as replaced_file:
                print("Update " + str(replaced_file_path))
                replaced_file.write(''.join(replaced_lines))
