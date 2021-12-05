@echo off

echo %EXPERIMENTAL%

set TOOLSET=v142
set LIB=demanglers

echo %TOOLSET%

set SRC=.build\%TOOLSET%
set DST=%EXPERIMENTAL%\%LIB%\%TOOLSET%

echo %SRC%\Win32\Debug\%LIB%.dll
echo %DST%\Win32\Debug\


mkdir %DST%\Win32\Debug\ > nul 2> nul
pushd %SRC%\Win32\Debug\
for %%I in (%LIB%.dll demanglers_imp.lib %LIB%.pdb) do copy %%I %DST%\Win32\Debug\
popd
pushd %SRC%\Win32\DebugStatic\
for %%I in (%LIB%.lib) do copy %%I %DST%\Win32\Debug\
popd
mkdir %DST%\Win32\Release\ > nul 2> nul
pushd %SRC%\Win32\Release\
for %%I in (%LIB%.dll %LIB%_imp.lib %LIB%.pdb) do copy %%I %DST%\Win32\Release\
popd
pushd %SRC%\Win32\ReleaseStatic\
for %%I in (%LIB%.lib) do copy %%I %DST%\Win32\Release\
popd





