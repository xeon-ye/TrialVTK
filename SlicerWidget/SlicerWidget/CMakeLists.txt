project(SlicerWidget)

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

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

# Set your files and resources here
set( Srcs
  main.cxx
  app.cxx
  datamanager.cpp
  filedialog.cxx
)

set( Hdrs app.hpp
  )

set( MOC_Hdrs app.hpp )

set( UIs app.ui datamanager.ui)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(SlicerWidget ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs} .clang-format)

target_link_libraries(SlicerWidget Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(SlicerWidget ${VTK_LIBRARIES})

if (WIN32)
  #get_target_property(_outdir SlicerWidget RUNTIME_OUTPUT_DIRECTORY)
  #message(${_outdir})
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/SlicerWidget.bat.in" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/SlicerWidget.bat" @ONLY)
endif()

include(deploy_vtk_windows)
# windeployqt(SlicerWidget ${CMAKE_CURRENT_BINARY_DIR})
windeployvtk(SlicerWidget ${CMAKE_CURRENT_BINARY_DIR})
