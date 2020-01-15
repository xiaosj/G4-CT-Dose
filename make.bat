echo OFF
set src=%~dp0
if not exist bin mkdir bin
cd bin
echo Generating cmake files ......
cmake -DGeant4_DIR=%G4Lib_Dir% %src%
echo
echo Building ...
cmake --build . --config Release
cd ..
