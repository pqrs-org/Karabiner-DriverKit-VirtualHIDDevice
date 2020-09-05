#!/usr/bin/python3

import os
import re
import sys
from pathlib import Path
from itertools import chain

topDirectory = Path(__file__).resolve(True).parents[1]

with topDirectory.joinpath('version').open() as versionFile, topDirectory.joinpath('driver-version').open() as driverVersionFile:
    version = versionFile.readline().strip()
    versionNumber = 0
    for v in version.split('.'):
        versionNumber = versionNumber * 100 + int(v)

    driverVersion = driverVersionFile.readline().strip()
    driverVersionNumber = 0
    for v in driverVersion.split('.'):
        driverVersionNumber = driverVersionNumber * 100 + int(v)

    for templateFilePath in chain(topDirectory.rglob('*.hpp.in'),
                                  topDirectory.rglob('*.plist.in'),
                                  topDirectory.rglob('*.xml.in')):
        replacedFilePath = Path(re.sub(r'\.in$', '', str(templateFilePath)))
        needsUpdate = False

        with templateFilePath.open('r') as templateFile:
            templateLines = templateFile.readlines()
            replacedLines = []

            if replacedFilePath.exists():
                with replacedFilePath.open('r') as replacedFile:
                    replacedLines = replacedFile.readlines()
                    while len(replacedLines) < len(templateLines):
                        replacedLines.append('')
            else:
                replacedLines = templateLines

            for index, templateLine in enumerate(templateLines):
                line = templateLine
                line = line.replace('@VERSION@', version)
                line = line.replace('@VERSION_NUMBER@', str(versionNumber))
                line = line.replace('@DRIVER_VERSION@', driverVersion)
                line = line.replace('@DRIVER_VERSION_NUMBER@', str(driverVersionNumber))

                if (replacedLines[index] != line):
                    needsUpdate = True
                    replacedLines[index] = line

        if needsUpdate:
            with replacedFilePath.open('w') as replacedFile:
                print("Update " + str(replacedFilePath))
                replacedFile.write(''.join(replacedLines))
