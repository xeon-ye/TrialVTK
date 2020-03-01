cmake_minimum_required(VERSION 3.9.5)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
else()
  find_package(ItkVtkGlue REQUIRED)
  include(${ItkVtkGlue_USE_FILE})
  set(Glue ItkVtkGlue)
endif()

add_executable(MIRegistration MACOSX_BUNDLE main.cpp)

add_executable(MIRegistration4 MACOSX_BUNDLE mattes.cpp)
target_link_libraries(MIRegistration4
  ${Glue}  ${VTK_LIBRARIES} ${ITK_LIBRARIES})

if (WIN32)
 set(VTK_RUNTIME_DEPENDENCIES
   vtkCommonColor-8.2.dll
   vtkCommonCore-8.2.dll
   vtkCommonComputationalGeometry-8.2.dll
   vtkCommonCore-8.2.dll
   vtkCommonDataModel-8.2.dll
   vtkCommonExecutionModel-8.2.dll
   vtkCommonMath-8.2.dll
   vtkCommonMisc-8.2.dll
   vtkCommonSystem-8.2.dll
   vtkCommonTransforms-8.2.dll
   vtkDICOMParser-8.2.dll
   vtkFiltersCore-8.2.dll
   vtkFiltersExtraction-8.2.dll
   vtkFiltersGeneral-8.2.dll
   vtkFiltersGeometry-8.2.dll
   vtkFiltersSources-8.2.dll
   vtkFiltersStatistics-8.2.dll
   vtkfreetype-8.2.dll
   vtkglew-8.2.dll
   vtkImagingCore-8.2.dll
   vtkImagingFourier-8.2.dll
   vtkInteractionStyle-8.2.dll
   vtkIOImage-8.2.dll
   vtkjpeg-8.2.dll
   vtkmetaio-8.2.dll
   vtkpng-8.2.dll
   vtkRenderingCore-8.2.dll
   vtkRenderingFreeType-8.2.dll
   vtkRenderingOpenGL2-8.2.dll
   vtksys-8.2.dll
   vtktiff-8.2.dll
   vtkzlib-8.2.dll
 )
 set(VTK_RUNTIME_LIBRARIES)
 foreach(DLL ${VTK_RUNTIME_DEPENDENCIES})
   set(FOUND_DLL "FOUND_DLL-NOTFOUND")
   find_file(FOUND_DLL
     ${DLL}
     PATHS "e:/code/bin/VTK/bin/Release"
     NO_DEFAULT_PATH)
   if (FOUND_DLL)
     list(APPEND VTK_RUNTIME_LIBRARIES ${FOUND_DLL})
   endif()
 endforeach(DLL)

  foreach(DLL ${VTK_RUNTIME_LIBRARIES})
    add_custom_command(TARGET MIRegistration POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${DLL}"
      "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION:MIRegistration>")
  endforeach(DLL)

endif()
