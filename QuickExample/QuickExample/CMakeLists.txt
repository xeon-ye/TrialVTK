cmake_minimum_required(VERSION 3.13.4)

set(MODULE_NAME QuickExample)

find_package(Qt5 COMPONENTS Quick Widgets Qml QuickControls2 OpenGL Concurrent REQUIRED)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

set(CMAKE_AUTOMOC ON)

find_package(VTK 8.2 REQUIRED NO_MODULE)
include(${VTK_USE_FILE})

add_subdirectory(res)

set(PUBLIC_HEADER_FILES
  main.hpp
  )

set(PRIVATE_HEADER_FILES
  application.hpp
  )

set(HEADERS
  ${HEADERS}
  ${PRIVATE_HEADER_FILES}
  )

set(SOURCE_FILES
  main.cpp
  #application.cpp
)

add_definitions(-DSPS_TRACE)
if (UNIX AND NOT WIN32 AND NOT CYGWIN)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../strace")
  set(HEADERS "${HEADERS}" "../../strace/strace.hpp")
  set(SOURCE_FILES "${SOURCE_FILES}" "../../strace/strace.cpp")
endif()


# Configuration file is often included in public headers - hence public
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

# Qt Resources
qt5_add_resources(RESOURCES res/qml.qrc)

if (WIN32)
  add_executable(${MODULE_NAME} WIN32 ${HEADERS} ${SOURCE_FILES} ${RESOURCES})
else()
  add_executable(${MODULE_NAME} ${HEADERS} ${SOURCE_FILES} ${RESOURCES})
endif()

target_include_directories(${MODULE_NAME}
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # for config.h
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

#target_link_libraries(${MODULE_NAME} viewcore)
target_link_libraries(${MODULE_NAME} Qt5::Quick Qt5::Widgets Qt5::Qml Qt5::QuickControls2 ${VTK_LIBRARIES})

target_link_libraries(${MODULE_NAME} viewcore)

#target_link_libraries(${MODULE_NAME} ${VTK_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Quick Qt5::Qml Qt5::Concurrent)


set(QT_QML_DIR "${Qt5Core_DIR}/../../../qml")

if (WIN32)
  # Add Qt release or debug DLLs to output directory
  get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
  set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")
  set(QT5_QTENV2_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/qtenv2.bat")
  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    DEPENDS ${TOOL_WINDEPLOYQT} ${TOOL_QTENV2}
    COMMAND ${TOOL_QTENV2}
    COMMAND ${TOOL_WINDEPLOYQT} --qmldir ${QT_QML_DIR} $<TARGET_FILE:${MODULE_NAME}> $<$<CONFIG:Release>:--release>$<$<NOT:$<CONFIG:Release>>:--debug> --no-compiler-runtime
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

  # Or alternatively
  # COMMAND ${CMAKE_COMMAND} -E copy_if_different
  #   $<TARGET_FILE:Qt5::Core>
  #   $<TARGET_FILE:Qt5::Widgets>
endif()
