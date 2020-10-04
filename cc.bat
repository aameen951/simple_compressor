@ECHO OFF

if not exist build mkdir build

pushd build
%VC_CL_EXE% /nologo /Zi /O2 ..\src\simple_compressor.cpp
popd