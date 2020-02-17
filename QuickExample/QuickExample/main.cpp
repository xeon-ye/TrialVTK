/**
 * @file   main.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Feb 17 21:06:03 2020
 *
 * @brief
 *
 * Copyright 2020 Jens Munk Hansen
 *
 */
#include <cstdlib>

// Example of a public header
#include <QuickExample/main.hpp>

// Example of a private header
#include <QuickExample/application.hpp>

int main(int argc, char* argv[]) {
#ifdef __linux
  // Fixes decimal point issue in vtkSTLReader
  putenv(const_cast<char*>("LC_NUMERIC=C"));

  // putenv(const_cast<char*>("MESA_GL_VERSION_OVERRIDE=3.2")); // Check if necessary
  putenv(const_cast<char*>("QML_BAD_GUI_RENDER_LOOP=1"));
  putenv(const_cast<char*>("QML_USE_GLYPHCACHE_WORKAROUND=1"));
#elif _MSC_VER
  // Fixes decimal point issue in vtkSTLReader
  _putenv(const_cast<char*>("LC_NUMERIC=C"));

  _putenv(const_cast<char*>("QML_BAD_GUI_RENDER_LOOP=1"));
  _putenv(const_cast<char*>("QT_AUTO_SCREEN_SCALE_FACTOR=1"));
  _putenv(const_cast<char*>("QML_USE_GLYPHCACHE_WORKAROUND=1"));
#endif

  return Quick::Application::Execute(argc, argv);
}

