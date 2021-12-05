@echo off

rem USAGE: apply_patch [-f]	; -f: force patching and removing files (otherwise no patch!)

if NOT EXIST adc.patch.xxd goto ERROR

utils\xxd.exe -r -p adc.patch.xxd > adc.patch.zip

utils\unzip.exe adc.patch.zip

IF "%1"=="-s" GOTO SAFE
IF NOT "%1"=="-f" GOTO SKIP

rem utils\patch.exe -p0 -i adc.patch
utils\mypatch.exe adc.patch
GOTO CLEAN

:SAFE

utils\mypatch.exe -s adc.patch
goto SKIP

:CLEAN

utils\rm adc.patch
utils\rm adc.patch.zip
GOTO EXIT

:SKIP
echo Skipping patch... (-f missing)
GOTO EXIT

:ERROR
echo error

:EXIT