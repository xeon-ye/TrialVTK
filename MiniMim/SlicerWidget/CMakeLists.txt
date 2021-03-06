project(MiniMim)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
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
  datamanager.cpp
)

set( Hdrs datamanager.h
  )

set( MOC_Hdrs datamanager.h )

set( UIs datamanager.ui)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_library(SlicerWidget2 ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

target_link_libraries(SlicerWidget2 Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(SlicerWidget2 ${VTK_LIBRARIES})
