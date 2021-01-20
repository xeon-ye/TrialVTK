@echo off

cmake ..\VTK\ -DModule_vtkGUISupportQt=ON -DModule_vtkGUISupportQtOpenGL=ON -DModule_vtkImagingOpenGL2=ON -DModule_vtkRenderingQt=ON -DModule_vtkViewsQt=ON -DVTK_Group_Qt=ON -DVTK_GroupRendering=ON -DVTK_Group_StandAlone=ON
