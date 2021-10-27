@echo off
pushd %~dp0
RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 128 %CD%\MyDriver1.inf
popd