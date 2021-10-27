@echo off
pushd %~dp0

:loop
sc stop MyDriver1

RunDLL32.Exe SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 %CD%\MyDriver1.inf

xcopy /y "\\Pandora\SHARED\MyDriver1*" "c:\Users\test\Desktop\wfp\"

RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 128 %CD%\MyDriver1.inf

sc start MyDriver1
sc query MyDriver1

pause
goto loop

popd