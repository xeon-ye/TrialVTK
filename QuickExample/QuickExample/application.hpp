/**
 * @file   application.hpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Feb  6 10:24:09 2020
 *
 * @brief
 *
 * Copyright 2020
 *
 */

#pragma once

#include <QtGlobal>

class QQmlApplicationEngine;

namespace Quick {

class Application {
private:
  static Application* pInstance;
  QQmlApplicationEngine* m_pEngine;
  void Initialize();
  static void HandleMessage(QtMsgType type,
                            const QMessageLogContext& context,
                            const QString& msg);
  Application();
  ~Application();
public:
  static int Execute(int argc, char* argv[]);
};
}  // namespace Quick

