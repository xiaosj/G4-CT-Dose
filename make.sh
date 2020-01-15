export src=$(pwd)
mkdir build
cd build
cmake -DGeant4_DIR=/usr/local/Cellar/geant4/10.5.1/lib/Geant4-10.5.1/ $src
make -j2