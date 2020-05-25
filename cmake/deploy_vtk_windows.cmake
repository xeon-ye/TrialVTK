find_package(VTK 8.2 REQUIRED NO_MODULE)

function(windeployvtk target directory)
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
    vtkexpat-8.2.dll
    vtkFiltersCore-8.2.dll
    vtkFiltersExtraction-8.2.dll
    vtkFiltersGeneral-8.2.dll
    vtkFiltersGeometry-8.2.dll
    vtkFiltersHybrid-8.2.dll
    vtkFiltersModeling-8.2.dll
    vtkFiltersPoints-8.2.dll
    vtkFiltersSources-8.2.dll
    vtkFiltersStatistics-8.2.dll
    vtkFiltersTexture-8.2.dll
    vtkfreetype-8.2.dll
    vtkglew-8.2.dll
    vtkGUISupportQt-8.2.dll
    vtkInteractionImage-8.2.dll
    vtkImagingCore-8.2.dll
    vtkImagingColor-8.2.dll
    vtkImagingGeneral-8.2.dll
    vtkImagingFourier-8.2.dll
    vtkImagingMath-8.2.dll
    vtkImagingHybrid-8.2.dll
    vtkImagingSources-8.2.dll
    vtkInteractionStyle-8.2.dll
    vtkInteractionWidgets-8.2.dll
    vtkIOCore-8.2.dll
    vtkIOImage-8.2.dll
    vtkIOLegacy-8.2.dll
    vtkIOGeometry-8.2.dll
    vtkIOXML-8.2.dll
    vtkIOXMLParser-8.2.dll
    vtkjpeg-8.2.dll
    vtkmetaio-8.2.dll
    vtkpng-8.2.dll
    vtkOpenGL2-8.2.dll
    vtkIOPLY-8.2.dll
    vtkRenderingCore-8.2.dll
    vtkRenderingExternal-8.2.dll
    vtkRenderingFreeType-8.2.dll
    vtkRenderingOpenGL2-8.2.dll
    vtkRenderingAnnotation-8.2.dll
    vtkRenderingVolume-8.2.dll
    vtkRenderingVolumeOpenGL2-8.2.dll
    vtkRenderingGL2PSOpenGL2-8.2.dll
    vtkdoubleconversion-8.2.dll
    vtkgl2ps-8.2.dll
    vtksys-8.2.dll
    vtklzma-8.2.dll
    vtklz4-8.2.dll
    vtktiff-8.2.dll
    vtkzlib-8.2.dll
  )
  set(VTK_RUNTIME_LIBRARIES)
  set(VTK_RUNTIME_LIBRARIES_DEBUG)

  # TODO: Check if installed, then VTK_DIR/../../../bin

  foreach(DLL ${VTK_RUNTIME_DEPENDENCIES})
    set(FOUND_DLL "FOUND_DLL-NOTFOUND")
    find_file(FOUND_DLL
      ${DLL}
      PATHS "${VTK_DIR}/bin/Release"
      NO_DEFAULT_PATH)
    if (FOUND_DLL)
      list(APPEND VTK_RUNTIME_LIBRARIES ${FOUND_DLL})
    endif()
  endforeach(DLL)

  foreach(DLL ${VTK_RUNTIME_DEPENDENCIES})
    set(FOUND_DLL "FOUND_DLL-NOTFOUND")
    find_file(FOUND_DLL
      ${DLL}
      PATHS "${VTK_DIR}/bin/Debug"
      NO_DEFAULT_PATH)
    if (FOUND_DLL)
      list(APPEND VTK_RUNTIME_LIBRARIES_DEBUG ${FOUND_DLL})
    endif()
  endforeach(DLL)

  foreach(DLL ${VTK_RUNTIME_LIBRARIES})
    add_custom_command(TARGET "${target}" POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E
      $<$<NOT:$<CONFIG:Release>>:echo>
      $<$<NOT:$<CONFIG:Release>>:"omitted">
      copy_if_different "${DLL}" "${directory}")
  endforeach(DLL)

  # Consider using
  # string(REPLACE "Release" "Debug" VTK_RUNTIME_DEPENDENCIES "${VTK_RUNTIME_DEPENDENCIES}")


  foreach(DLL ${VTK_RUNTIME_LIBRARIES_DEBUG})
    add_custom_command(TARGET "${target}" POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E
      $<$<CONFIG:Release>:echo>
      $<$<CONFIG:Release>:"omitted">
      copy_if_different "${DLL}" "${directory}")
  endforeach(DLL)
endfunction()
