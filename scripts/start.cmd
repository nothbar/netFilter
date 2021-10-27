@echo off
pushd %~dp0
net start MyDriver1
sc start MyDriver1
sc query MyDriver1
popd