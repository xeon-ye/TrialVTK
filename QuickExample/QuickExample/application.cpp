/**
 * @file   application.cpp
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu Feb  6 10:25:32 2020
 *
 * @brief
 *
 * Copyright 2020
 */

#include <application.hpp>

#include <QWindow>
#include <QApplication>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QSurfaceFormat>
#include <QDebug>

#include <memory>

#include <viewcore/view.hpp>
#include <viewcore/renderer.hpp>
#include "QVTKOpenGLWidget.h"

// FIX ME
//#include <viewcore/src/view.hpp>

namespace Quick {

Application* Application::pInstance = nullptr;

Application::Application() : m_pEngine(nullptr) {}

void Application::Initialize() {
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  QApplication::setOrganizationName("SexySoft");
  QApplication::setOrganizationDomain("XXX");
  QApplication::setApplicationName("VTKGuiTemplate");

  m_pEngine = new QQmlApplicationEngine();

  auto context = m_pEngine->rootContext();

  /*
  context->setContextProperty("App", Controller::instance);
  context->setContextProperty("SampleData", SampleData::Controller::GetInstance());
  */
  // Register simple VTK type
  //int retval = qmlRegisterType<Quick::Vtk::View>("QtVTK", 1, 0, "VtkFboItem");

  // Register QML types
  int typeId = qmlRegisterType<QVTKFramebufferObjectItem>("ViewCore", 1, 0, "VtkFboItem");

  qDebug() << "typeId: " << typeId;

  m_pEngine->load(QUrl(QStringLiteral("qrc:/res/qml/main.qml")));

  auto rootObject = m_pEngine->rootObjects().at(0);

  m_vtkFboItem = rootObject->findChild<QVTKFramebufferObjectItem*>("vtkFboItem");

  // Give the vtkFboItem reference to the Application
  if (m_vtkFboItem) {
    qDebug() << "Application::Application: setting vtkFboItem to Application";

    m_vtkFboItem->setProcessingEngine(m_processingEngine);

    connect(m_vtkFboItem, &QVTKFramebufferObjectItem::rendererInitialized, this, &Application::startApplication);
    connect(m_vtkFboItem, &QVTKFramebufferObjectItem::isModelSelectedChanged, this, &Application::isModelSelectedChanged);
    connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionXChanged, this, &Application::selectedModelPositionXChanged);
    connect(m_vtkFboItem, &QVTKFramebufferObjectItem::selectedModelPositionYChanged, this, &Application::selectedModelPositionYChanged);
  } else {
    qCritical() << "Application::Application: Unable to get vtkFboItem instance";
    return;
  }


  auto pWindow = static_cast<QWindow*>(rootObject);
#if 0
  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setDepthBufferSize(1);
  format.setStencilBufferSize(1);
  format.setProfile(QSurfaceFormat::CoreProfile);

  pWindow->setFormat(format);
#endif
  pWindow->showMaximized();
}


void Application::HandleMessage(QtMsgType type,
                                const QMessageLogContext& context,
                                const QString& msg) {
  qDebug() << "  > " << msg;
}

int Application::Execute(int argc, char* argv[]) {

  // Some attributes must be set before creating a core application
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication application(argc, argv);

  if (!Application::pInstance) {
    Application::pInstance = new Application();
    pInstance->Initialize();
    return application.exec();
  }

  return -1;
}
Application::~Application() {
  if (m_pEngine) {
    delete m_pEngine;
    m_pEngine = nullptr;
  }
}
}  // namespace Quick


