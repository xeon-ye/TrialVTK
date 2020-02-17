cmake_minimum_required(VERSION 3.13.4)

find_package(Qt5 COMPONENTS Quick Widgets Qml QuickControls2 OpenGL Concurrent REQUIRED)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

find_package(VTK 8.2 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

set(SOURCE_FILES
  fbo.cxx
  renderer.cxx
)

set(HEADER_FILES
  fbo.hpp
  renderer.hpp
)

add_library(viewcore SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(viewcore Qt5::Quick Qt5::Widgets Qt5::Qml Qt5::QuickControls2 ${VTK_LIBRARIES} ${SYSTEM_LIBS})
