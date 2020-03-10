cmake_minimum_required(VERSION 3.13.4)

project(TrialVTK C CXX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# c++14 for all targets
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add path for custom modules
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(deploy_qt_windows)
include(deploy_vtk_windows)

#add_subdirectory(WidgetExample)
#add_subdirectory(QuickExample)
<<<<<<< HEAD
add_subdirectory(SlicerWidget)
#add_subdirectory(QuickDockExample)
#add_subdirectory(Registration)
=======
#add_subdirectory(SlicerWidget)
add_subdirectory(Registration)
add_subdirectory(Picking)
>>>>>>> c3c9e6f884af0ea9d51a208a279f8f4e22b3397a
#add_subdirectory(Cone)
