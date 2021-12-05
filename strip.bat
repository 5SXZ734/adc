@echo OFF
if exist CVS goto WRONG_DIR

IF EXIST 3rdparty RMDIR /S /Q 3rdparty
IF EXIST doc RMDIR /S /Q doc
IF EXIST tests RMDIR /S /Q tests
IF EXIST utils RMDIR /S /Q utils
IF EXIST src\trash RMDIR /S /Q src\trash
goto EXIT


:WRONG_DIR
echo Wrong Directory

:EXIT