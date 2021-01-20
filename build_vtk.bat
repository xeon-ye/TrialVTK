@echo off

call \Qt\Qt5.12.9\5.12.9\msvc2017_64\bin\qtenv2.bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Ent
erprise\VC\Auxiliary\Build\vcvarsall.bat" x64

cmake ..\VTK\ -DModule_vtkGUISupportQt=ON -DModule_vtkGUISupportQtOpenGL=ON -DModule_vtkImagingOpenGL2=ON -DModule_vtkRenderingQt=ON -DModule_vtkViewsQt=ON -DVTK_Group_Qt=ON -DVTK_GroupRendering=ON -DVTK_Group_StandAlone=ON -DBUILD_TESTING=OFF

MSBuild.exe VTK.sln /p:Configuration=Release 
