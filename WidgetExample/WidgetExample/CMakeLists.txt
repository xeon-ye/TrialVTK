project(WidgetExample)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

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
  FileDialog.cxx
)

set( Hdrs app.hpp
  FileDialog.hpp
  )

set( MOC_Hdrs app.hpp )

set( UIs app.ui )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(WidgetExample ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

target_link_libraries(WidgetExample Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(WidgetExample ${VTK_LIBRARIES})

# windeployqt(WidgetExample ${CMAKE_CURRENT_BINARY_DIR})
