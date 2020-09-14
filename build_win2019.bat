@echo off

pushd %~dp0

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Variable used in this script - name is arbitrary
set QTDIR=c:/Qt/5.15.0/msvc2019_64
mkdir build

set CMAKE_DIR="c:\Program Files\CMake\bin"

set VTK_DIR=E:/code/bin/VTK
REM set VTK_DIR=c:/ArtifactoryCache/vtk.msvc-v141.x64/dist

%CMAKE_DIR%\cmake -H%~dp0 -B%~dp0\build -G "Visual Studio 16 2019" -A "x64" -T v141 -DCMAKE_PREFIX_PATH=%VTK_DIR%;%QTDIR%

REM Create solution
cd %~dp0\build

REM Build solution
msbuild TrialVTK.sln /p:Configuration=Release

cd %~dp0\build\Release

REM set CURRENTDRIVE=%CD:~0,2%
REM %QTDIR%\bin\qtenv2.bat
REM cd /D %CURRENTDRIVE%%~dp0\build\Release
REM %QTDIR%\bin\windeployqt.exe Registration.exe

cd %~dp0
