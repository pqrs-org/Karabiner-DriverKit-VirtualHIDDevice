#!/usr/bin/python3

import os
import re
import sys
from pathlib import Path
from itertools import chain

topDirectory = Path(__file__).resolve(True).parents[1]

with topDirectory.joinpath('version').open() as f:
    version = f.readline().strip()

    for templateFilePath in chain(topDirectory.rglob('Info.plist.in'),
                                  topDirectory.rglob('version.hpp.in')):
        replacedFilePath = Path(re.sub(r'\.in$', '', str(templateFilePath)))
        needsUpdate = False

        with templateFilePath.open('r') as templateFile:
            templateLines = templateFile.readlines()
            replacedLines = []

            if replacedFilePath.exists():
                with replacedFilePath.open('r') as replacedFile:
                    replacedLines = replacedFile.readlines()
            else:
                replacedLines = templateLines

            for index, templateLine in enumerate(templateLines):
                if re.search(r'@VERSION@', templateLine):
                    line = templateLine.replace('@VERSION@', version)
                    if (replacedLines[index] != line):
                        needsUpdate = True
                        replacedLines[index] = line

        if needsUpdate:
            with replacedFilePath.open('w') as replacedFile:
                print("Update " + str(replacedFilePath))
                replacedFile.write(''.join(replacedLines))
