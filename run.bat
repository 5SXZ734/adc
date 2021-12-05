@echo off

SET DATE0=%date:~-4%_%date:~4,2%_%date:~7,2%
SET TIME12=0%time:~1,1%_%time:~3,2%_%time:~6,2% 
SET TIME24=%time:~0,2%_%time:~3,2%_%time:~6,2%
SET dtStamp9= %date:~-4%%date:~4,2%%date:~7,2%_0%time:~1,1%%time:~3,2%%time:~6,2% 
SET dtStamp24=%date:~-4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
SET HOUR=%time:~0,2%
if "%HOUR:~0,1%" == " " (SET TIME0=%TIME12%) else (SET TIME0=%TIME24%)

rem ECHO %DATE0%
rem ECHO %TIME12%

SET Toolset=v142
SET Platform=%1
SET Config=Release
SHIFT

:loop
IF NOT "%1"=="" (
    IF "%1"=="-d" (
        SET Config=Debug
        SHIFT
    )
    SHIFT
    GOTO :loop
)



SET CURDIR=%CD%
rem SET TARGET=%CD%\.build\%Toolset%\%Platform%\%Config%\adc.exe
SET TARGET=%CD%\_build\%Config%\adc.exe
SET SRCDIR=%CD%\tests
rem SET OUTDIR=%CD%\.tests\%DATE0%\%TIME0%
SET OUTDIR=%CD%\.tests
SET "FLAGS=-b -t -nlogo -naid -s"

rmdir %OUTDIR% /s /q
mkdir %OUTDIR%

rem echo %TARGET%
rem echo %SRCDIR%
rem echo %OUTDIR%
rem PAUSE

cd %OUTDIR%

IF "%2"=="migalley" GOTO %2
IF "%2"=="flanker" GOTO %2

mkdir %OUTDIR%\notepad
@echo on
%TARGET% %FLAGS% %SRCDIR%\notepad\ex00 -o notepad\ex00.out
%TARGET% %FLAGS% %SRCDIR%\notepad\ex01 -o notepad\ex01.out
%TARGET% %FLAGS% %SRCDIR%\notepad\ex02 -o notepad\ex02.out
%TARGET% %FLAGS% %SRCDIR%\notepad\ex03 -o notepad\ex03.out

:migalley
mkdir %OUTDIR%\mig
@echo on
%TARGET% %FLAGS% %SRCDIR%\mig\ex01 -o mig\ex01.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex02 -o mig\ex02.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex03 -o mig\ex03.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex04 -o mig\ex04.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex05 -o mig\ex05.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex06 -o mig\ex06.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex07 -o mig\ex07.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex09 -o mig\ex09.out
%TARGET% %FLAGS% %SRCDIR%\mig\ex10 -o mig\ex10.out
rem %TARGET% %FLAGS% %SRCDIR%\mig\ex13 -o mig\ex13.out

:flanker
mkdir %OUTDIR%\flanker
@echo on
%TARGET% %FLAGS% %SRCDIR%\flanker\ex00 -o flanker\ex00.out
%TARGET% %FLAGS% %SRCDIR%\flanker\ex01 -o flanker\ex01.out
%TARGET% %FLAGS% %SRCDIR%\flanker\ex02 -o flanker\ex02.out
%TARGET% %FLAGS% %SRCDIR%\flanker\ex03 -o flanker\ex03.out

:cfs2
mkdir %OUTDIR%\cfs2
@echo on
%TARGET% %FLAGS% %SRCDIR%\cfs2\ex01 -o cfs2\ex01.out
%TARGET% %FLAGS% %SRCDIR%\cfs2\ex02 -o cfs2\ex02.out
%TARGET% %FLAGS% %SRCDIR%\cfs2\ex03 -o cfs2\ex03.out

:snowman
mkdir %OUTDIR%\snowman
@echo on
%TARGET% %FLAGS% %SRCDIR%\snowman\ex01 -o snowman\ex01.out

:elf
mkdir %OUTDIR%\elf
@echo on
%TARGET% %FLAGS% %SRCDIR%\elf\ex01 -o elf\ex01.out
%TARGET% %FLAGS% %SRCDIR%\elf\ex01 -o elf\ex04.out

:pdb
mkdir %OUTDIR%\pdb
@echo on
%TARGET% %FLAGS% %SRCDIR%\pdb\ex01 -o pdb\ex01.out
%TARGET% %FLAGS% %SRCDIR%\pdb\ex03 -o pdb\ex03.out
%TARGET% %FLAGS% %SRCDIR%\pdb\ex05 -o pdb\ex05.out

:gtk
mkdir %OUTDIR%\gtk
@echo on
%TARGET% %FLAGS% %SRCDIR%\gtk\ex01 -o gtk\ex01.out


cd %CURDIR%

start .\utils\WinMerge\WinMergeU /r /e /s /wl /wr .tests golden

rem PAUSE




