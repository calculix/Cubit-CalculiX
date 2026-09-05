rm -r build/
cmake -S src -B build/
cd build/
make -j8
cd ..
