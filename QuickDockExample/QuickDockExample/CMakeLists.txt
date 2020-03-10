project(QuickDockExample)
set(MODULE_NAME QuickDockExample)

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
  datamanager.h
  )

set(HEADERS
  ${HEADERS}
  ${PRIVATE_HEADER_FILES}
  )

set(SOURCE_FILES
  main.cpp
  qmlplot.cpp
  datamanager.cpp
)

set( UIs datamanager.ui)

qt5_wrap_ui(UI_Srcs ${UIs})

# Qt Resources
qt5_add_resources(RESOURCES res/qml.qrc)

add_executable(QuickDockExample ${HEADERS} ${SOURCE_FILES} ${RESOURCES})

target_link_libraries(QuickDockExample Qt5::Quick Qt5::Widgets Qt5::Qml Qt5::QuickControls2 ${VTK_LIBRARIES})

set(QT_QML_DIR "${Qt5Core_DIR}/../../../qml")

if (WIN32)
  # Add Qt release or debug DLLs to output directory
  get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(QT5_WINDEPLOY_EXECUTABLE_DIR ${QT5_QMAKE_EXECUTABLE} PATH)
  set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOY_EXECUTABLE_DIR}/windeployqt.exe")
  set(QT5_QTENV2_EXECUTABLE "${QT5_WINDEPLOY_EXECUTABLE_DIR}/qtenv2.bat")
  message(${QT5_WINDEPLOYQT_EXECUTABLE})
  message(${QT5_QTENV2_EXECUTABLE})

  add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    DEPENDS ${QT5_QTENV2_EXECUTABLE} ${QT5_WINDEPLOYQT_EXECUTABLE}
    COMMAND ${QT5_QTENV2_EXECUTABLE}
    COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --qmldir=${QT_QML_DIR} $<$<CONFIG:Release>:--release>$<$<NOT:$<CONFIG:Release>>:--debug> --no-compiler-runtime $<TARGET_FILE:${MODULE_NAME}>
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

  # Or alternatively
  # COMMAND ${CMAKE_COMMAND} -E copy_if_different
  #   $<TARGET_FILE:Qt5::Core>
  #   $<TARGET_FILE:Qt5::Widgets>
endif()
