cmake -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_TOOLCHAIN_FILE=%PS5SDK%/cmake/toolchain-ps5.cmake .
ninja
