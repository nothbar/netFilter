@echo off
pushd %~dp0
fltmc unload MyDriver1
sc stop MyDriver1
sc query MyDriver1
popd