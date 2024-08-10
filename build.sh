#!/bin/bash

libs="-luser32 -lopengl32"
warnings="-Wno-writable-strings -Wno-format-security -Wno-writable-strings -Wno-deprecated-declarations"
includes="-Ithird_party -Ithird_party/Include"

clang++ $includes -g src/main.cpp -oengine.exe $libs $warnings