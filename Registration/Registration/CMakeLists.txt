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

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

# Set your files and resources here
set( Srcs
  main.cxx
  app.cxx
  filedialog.cxx
)

set( Hdrs app.hpp
  )

set( MOC_Hdrs app.hpp )

set( UIs app.ui)


# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(UI_Srcs ${UIs})

# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
add_executable(Registration ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})

target_link_libraries(Registration Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(Registration ${VTK_LIBRARIES})

include(deploy_vtk_windows)
# windeployqt(Registration ${CMAKE_CURRENT_BINARY_DIR})
windeployvtk(Registration ${CMAKE_CURRENT_BINARY_DIR})
