#!/bin/bash

timestamp=$( date +%s)

libs="-luser32 -lopengl32 -lgdi32 -lole32"
warnings="-Wno-writable-strings -Wno-format-security -Wno-writable-strings -Wno-deprecated-declarations -Wno-switch"
includes="-Ithird_party -Ithird_party/Include"

cleanup_files() {
    echo "Cleaning up unwanted files..."
    
    rm -f *.ilk
    echo "Removed .ilk files"
    
    rm -f *.lib
    echo "Removed .lib files"
    
    rm -f *.pdb
    echo "Removed .pdb files"
    
    rm -f *.exp
    echo "Removed .exp files"
    
    rm -f game_*
    echo "Removed old game files"
    
    echo "Cleanup completed. Only engine.exe and essential files remain."
}

if [[ "$(uname)" == "Linux" ]];then

    echo "Running on Linux"
    libs="-lX11 -lGL -lfreetype"
    outputFile=engine
    rm -f game_* #removing old game file
    clang++ -g src/libs/game.cpp -shared -fPIC -o game_$timestamp.so $libs $warnings
    mv game_$timestamp.so game.so
else
    echo "Running on Windows"
    libs="-luser32 -lopengl32 -lgdi32 -lole32"
    outputFile=engine.exe

    rm -f game_*
    clang++ -g "src/libs/game.cpp"  -shared -o game_$timestamp.dll $libs $warnings
    mv game_$timestamp.dll game.dll
fi

clang++ -g $includes src/main.cpp -o $outputFile $libs $warnings $defines 2> build_errors.txt
if [[ $? -ne 0 ]]; then
    echo "Build failed. Check build_errors.txt for details."
else
    echo "Build succeeded. Output file: $outputFile"
    
    cleanup_files
fi