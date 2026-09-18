cd build/
make -j8
cd ..
rm -r ~/Downloads/Coreform-Cubit-2026.9-rc+70210-Lin64/Coreform-Cubit-2026.9/bin/plugins/
cp -R build/ ~/Downloads/Coreform-Cubit-2026.9-rc+70210-Lin64/Coreform-Cubit-2026.9/bin/plugins/
