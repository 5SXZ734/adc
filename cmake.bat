@echo off

mkdir _build
if defined SS_MAKE_SITE ( attrib +h _build )
cd _build
cmake .. -G "Visual Studio 16 2019"
cd ..
