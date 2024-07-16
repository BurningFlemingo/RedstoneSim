A set and graph theoretic model of minecraft redstone, and a simulation using it.

## Building

1. call ```git clone git@github.com:burningflemingo/RedstoneSim --recursive```
2. update vcpkg.json with dependencys
3. install dependencys and bootstrap vcpkg with ```python scripts/bootstrap_vcpkg.py```
5. to build call python ```scripts/build.py platform-preset```  where the preset is defined in the CMakePresets.txt OR (to build for internal) call ```python helper_scripts/build_internal.py```
