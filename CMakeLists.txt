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

if (WIN32)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release")
endif()

#add_subdirectory(WidgetExample)
#add_subdirectory(QuickExample)
#add_subdirectory(QuickDockExample)
add_subdirectory(Registration)
<<<<<<< HEAD
=======
#add_subdirectory(Registration)
>>>>>>> e5285139633377c8c4835e38c5243aaef92e37f0
add_subdirectory(SlicerWidget)
add_subdirectory(MiniMim)
#add_subdirectory(Registration)
#add_subdirectory(Picking)
#add_subdirectory(Cone)
