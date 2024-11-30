#!/bin/bash

timestamp=$( date +%s)

libs="-luser32 -lopengl32 -lgdi32 -lole32"
warnings="-Wno-writable-strings -Wno-format-security -Wno-writable-strings -Wno-deprecated-declarations -Wno-switch"
includes="-Ithird_party -Ithird_party/Include"

if [[ "$(uname)" == "Linux" ]];then
    echo "Running on Linux"
    libs="-lX11 -lGL -lfreetype"
    outputFile=engine
    rm -f game_* #removing old game file
    clang++ -g src/libs/game.cpp -shared -fPIC -o game_$timestamp.so $libs $warnings
    mv game_$timestamp.so game.so
else
    echo "Running on Windows"
    libs="-luser32 -lopengl32 -lgdi32 -lole32 -Lthird_party/lib -lfreetype.lib"
    outputFile=engine.exe

    rm -f game_*
    clang++ -g "src/libs/game.cpp" -shared -o game_$timestamp.dll $warnings $defines
    mv game_$timestamp.dll game.dll
fi


clang++ -g $includes  src/main.cpp -o$outputFile $libs $warnings $defines