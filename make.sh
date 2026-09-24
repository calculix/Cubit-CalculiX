cd build/
make -j8
cd ..
rm -r ~/Downloads/Coreform-Cubit-2026.9/bin/plugins/
cp -R build/ ~/Downloads/Coreform-Cubit-2026.9/bin/plugins/
