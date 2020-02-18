#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlProperty>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>

#include <viewcore/config.h>
#include "Model.h"
#include "ProcessingEngine.h"
#include <viewcore/view.hpp>
#include <viewcore/renderer.hpp>
#include "QVTKOpenGLWidget.h"

#include "CanvasHandler.h"


CanvasHandler::CanvasHandler(int argc, char **argv) {
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());
  QApplication app(argc, argv);
  QQmlApplicationEngine engine;
#ifdef _WIN32
  engine.addImportPath(QTVTK_QML_DIR);
#endif
  app.setApplicationName("QtQuickVTK");
  app.setWindowIcon(QIcon(":/resources/bq.ico"));
  app.setOrganizationName("Sexy Soft");
  app.setOrganizationDomain("www.sexysoft.com");

  // Register QML types
  int retval = qmlRegisterType<View>("QtVTK", 1, 0, "VtkFboItem");

  // Create classes instances
  m_processingEngine = std::shared_ptr<ProcessingEngine>(new ProcessingEngine());

  // Expose C++ classes to QML
  QQmlContext* ctxt = engine.rootContext();

  ctxt->setContextProperty("canvasHandler", this);

  QQuickStyle::setStyle("Material");

  // Load main QML file
  engine.load(QUrl("qrc:/res/qml/main.qml"));

  // Get reference to the QVTKFramebufferObjectItem created in QML
  // We cannot use smart pointers because this object must be deleted by QML
  QObject *rootObject = engine.rootObjects().first();
  m_vtkFboItem = rootObject->findChild<View*>("vtkFboItem");

  // Give the vtkFboItem reference to the CanvasHandler
  if (m_vtkFboItem) {
    qDebug() << "CanvasHandler::CanvasHandler: setting vtkFboItem to CanvasHandler";

    m_vtkFboItem->setProcessingEngine(m_processingEngine);

    connect(m_vtkFboItem, &View::rendererInitialized, this, &CanvasHandler::startApplication);
    connect(m_vtkFboItem, &View::isModelSelectedChanged, this, &CanvasHandler::isModelSelectedChanged);
    connect(m_vtkFboItem, &View::selectedModelPositionXChanged, this, &CanvasHandler::selectedModelPositionXChanged);
    connect(m_vtkFboItem, &View::selectedModelPositionYChanged, this, &CanvasHandler::selectedModelPositionYChanged);
  } else {
    qCritical() << "CanvasHandler::CanvasHandler: Unable to get vtkFboItem instance";
    return;
  }

  const QString DEFAULT_MODEL_DIR_KEY("default_model_dir");

  QSettings MySettings;

  m_fileDialog = rootObject->findChild<QObject*>("myFileDialog");
  if (m_fileDialog) {
    QString tmp = MySettings.value(DEFAULT_MODEL_DIR_KEY).toString();
    m_fileDialog->setProperty("folder", QUrl::fromLocalFile(tmp));
  }

  int rc = app.exec();

  qDebug() << "CanvasHandler::CanvasHandler: Execution finished with return code:" << rc;
}


void CanvasHandler::startApplication() const {
  qDebug() << "CanvasHandler::startApplication()";

  disconnect(m_vtkFboItem, &View::rendererInitialized, this, &CanvasHandler::startApplication);
}


void CanvasHandler::openModel(const QUrl &path) const {
  qDebug() << "CanvasHandler::openModel():" << path;

  QUrl localFilePath;

  if (path.isLocalFile()) {
    // Remove the "file:///" if present
    localFilePath = path.toLocalFile();
  } else {
    localFilePath = path;
  }

  m_vtkFboItem->addModelFromFile(localFilePath);

  QDir currentDir = QFileInfo(localFilePath.toString()).absoluteDir();
  QString currentPath = currentDir.absolutePath();

  const QString DEFAULT_MODEL_DIR_KEY("default_model_dir");
  QSettings MySettings;
  MySettings.setValue(DEFAULT_MODEL_DIR_KEY, currentPath);

  m_fileDialog->setProperty("folder", QUrl::fromLocalFile(currentPath));
}

bool CanvasHandler::isModelExtensionValid(const QUrl &modelPath) const {
  if (modelPath.toString().toLower().endsWith(".stl") || modelPath.toString().toLower().endsWith(".obj")) {
    return true;
  }

  return false;
}


void CanvasHandler::mousePressEvent(const int button, const int screenX, const int screenY) const {
  qDebug() << "CanvasHandler::mousePressEvent()";

  m_vtkFboItem->selectModel(screenX, screenY);
}

void CanvasHandler::mouseMoveEvent(const int button, const int screenX, const int screenY) {
  if (!m_vtkFboItem->isModelSelected()) {
    return;
  }

  if (!m_draggingMouse) {
    m_draggingMouse = true;

    m_previousWorldX = m_vtkFboItem->getSelectedModelPositionX();
    m_previousWorldY = m_vtkFboItem->getSelectedModelPositionY();
  }

  CommandModelTranslate::TranslateParams_t translateParams;

  translateParams.screenX = screenX;
  translateParams.screenY = screenY;

  m_vtkFboItem->translateModel(translateParams, true);
}

void CanvasHandler::mouseReleaseEvent(const int button, const int screenX, const int screenY) {
  qDebug() << "CanvasHandler::mouseReleaseEvent()";

  if (!m_vtkFboItem->isModelSelected()) {
    return;
  }

  if (m_draggingMouse) {
    m_draggingMouse = false;

    CommandModelTranslate::TranslateParams_t translateParams;

    translateParams.screenX = screenX;
    translateParams.screenY = screenY;
    translateParams.previousPositionX = m_previousWorldX;
    translateParams.previousPositionY = m_previousWorldY;

    m_vtkFboItem->translateModel(translateParams, false);
  }
}


bool CanvasHandler::getIsModelSelected() const {
  // QVTKFramebufferObjectItem might not be initialized when QML loads
  if (!m_vtkFboItem) {
    return 0;
  }

  return m_vtkFboItem->isModelSelected();
}

double CanvasHandler::getSelectedModelPositionX() const {
  // QVTKFramebufferObjectItem might not be initialized when QML loads
  if (!m_vtkFboItem) {
    return 0;
  }

  return m_vtkFboItem->getSelectedModelPositionX();
}

double CanvasHandler::getSelectedModelPositionY() const {
  // QVTKFramebufferObjectItem might not be initialized when QML loads
  if (!m_vtkFboItem) {
    return 0;
  }

  return m_vtkFboItem->getSelectedModelPositionY();
}

void CanvasHandler::setModelsRepresentation(const int representationOption) {
  m_vtkFboItem->setModelsRepresentation(representationOption);
}

void CanvasHandler::setModelsOpacity(const double opacity) {
  m_vtkFboItem->setModelsOpacity(opacity);
}

void CanvasHandler::setGouraudInterpolation(const bool gouraudInterpolation) {
  m_vtkFboItem->setGouraudInterpolation(gouraudInterpolation);
}

void CanvasHandler::setModelColorR(const int colorR) {
  m_vtkFboItem->setModelColorR(colorR);
}

void CanvasHandler::setModelColorG(const int colorG) {
  m_vtkFboItem->setModelColorG(colorG);
}

void CanvasHandler::setModelColorB(const int colorB) {
  m_vtkFboItem->setModelColorB(colorB);
}

