set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Registration)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
#  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkFiltersSources
  vtkGUISupportQt
  vtkIOImage
  vtkInteractionImage
)

include(${VTK_USE_FILE})

set(INTEGRATED_SEGMENTATION 1)

# TODO: Find bug in the C++ code (currently disabled)
set(INTEGRATED_SURFACING 0)

if (INTEGRATED_SURFACING)
  if (NOT INTEGRATED_SEGMENTATION)
    message(FATAL_ERROR "Integrated surfacing requires integrated segmentation")
  endif()
endif()

if (INTEGRATED_SEGMENTATION)
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
endif()

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

# Set your files and resources here
set( Srcs
  main.cxx
  app.cxx
  filedialog.cxx
  runnable.cpp
  RangeSlider.cpp
  SegRunner.cpp
  SurfRunner.cpp
  whereami.c
  transformmodel.cpp
)

set( Hdrs
  app.hpp
  reslicecallback.h
  seedcallback.h
  whereami.h
  transformmodel.h
  )

set( MOC_Hdrs app.hpp )

set( UIs app.ui)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(UI_Srcs ${UIs})


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(Registration ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

target_link_libraries(Registration Qt5::Core Qt5::Gui Qt5::Widgets)

if (INTEGRATED_SEGMENTATION)
  target_link_libraries(Registration ${Glue}  ${VTK_LIBRARIES} ${ITK_LIBRARIES})
else()
  target_link_libraries(Registration ${VTK_LIBRARIES})
endif()

include(deploy_vtk_windows)
# windeployqt(Registration ${CMAKE_CURRENT_BINARY_DIR})
if (WIN32)
  windeployvtk(Registration $<$<CONFIG:Release>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}>$<$<NOT:$<CONFIG:Release>>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}>)
endif()
