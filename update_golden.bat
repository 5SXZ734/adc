@echo off

if not "%1"=="-f" goto CONFIRM

if not exist .tests goto NO_SRC_DIR
if exist golden rmdir golden /s /q
if exist .tests rename .tests golden
goto EXIT

:CONFIRM
echo REJECTED: please confirm (-f)
goto EXIT

:NO_SRC_DIR
echo ERROR: no source directory

:EXIT
