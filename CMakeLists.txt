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

add_subdirectory(WidgetExample)
add_subdirectory(QuickExample)
