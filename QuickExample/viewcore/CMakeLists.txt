cmake_minimum_required(3.13.4)

find_package(Qt5 COMPONENTS Quick Widgets Qml QuickControls2 OpenGL Concurrent REQUIRED)

set(SOURCE_FILES
  fbo.cxx
  renderer.cxx
)

set(HEADER_FILES
  fbo.hpp
  renderer.hpp
)
