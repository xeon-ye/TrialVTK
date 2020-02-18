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
  view.cxx
  renderer.cxx
  CommandModelTranslate.cpp
  CommandModel.cpp
  CommandModelAdd.cpp
  Model.cpp
  ProcessingEngine.cpp
  CanvasHandler.cpp
)

set(HEADER_FILES
  view.hpp
  renderer.hpp
  CommandModelTranslate.h
  CommandModel.h
  CommandModelAdd.h
  Model.h
  ProcessingEngine.h
  CanvasHandler.h
)

set(QTVTK_QML_DIR "${Qt5Core_DIR}/../../../qml")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

add_library(viewcore SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(viewcore Qt5::Quick Qt5::Widgets Qt5::Qml Qt5::QuickControls2 ${VTK_LIBRARIES} ${SYSTEM_LIBS})
