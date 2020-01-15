echo OFF
set src=%~dp0
if not exist bin mkdir bin
cd bin
echo Generating cmake files ......
cmake -DGeant4_DIR="C:\Users\xiaosj\geant4\geant4_10_05\lib\Geant4-10.5.0" %src%
echo
echo Building ...
cmake --build . --config Release
cd ..
