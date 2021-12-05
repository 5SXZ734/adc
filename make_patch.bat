@echo off


rem if "%1"=="" goto ERROR

utils\diff -rnd -x CVS -x msvc* ..\2h\adc\src src > adc.patch

utils\zip adc.patch.zip adc.patch

utils\rm adc.patch

utils\xxd.exe -ps adc.patch.zip > adc.patch.xxd

utils\rm adc.patch.zip

goto EXIT

rem :ERROR
rem echo missing original dir must be in ..\2h)

:EXIT