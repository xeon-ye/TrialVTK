#include "CommandModel.h"
#include "CommandModelAdd.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include <viewcore/view.hpp>
#include <viewcore/renderer.hpp>


View::View() {
  m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  m_lastMouseButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  m_lastMouseMove = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  m_lastMouseWheel = std::make_shared<QWheelEvent>(QPointF(0,0), 0, Qt::NoButton, Qt::NoModifier, Qt::Vertical);

  this->setMirrorVertically(true); // QtQuick and OpenGL have opposite Y-Axis directions

  setAcceptedMouseButtons(Qt::RightButton);
}


QQuickFramebufferObject::Renderer *View::createRenderer() const {
  return new QVTKFramebufferObjectRenderer();
}

void View::setVtkFboRenderer(QVTKFramebufferObjectRenderer* renderer) {
  qDebug() << "QVTKFramebufferObjectItem::setVtkFboRenderer";

  m_vtkFboRenderer = renderer;

  connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::isModelSelectedChanged, this, &View::isModelSelectedChanged);
  connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionXChanged, this, &View::selectedModelPositionXChanged);
  connect(m_vtkFboRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionYChanged, this, &View::selectedModelPositionYChanged);

  m_vtkFboRenderer->setProcessingEngine(m_processingEngine);
}

bool View::isInitialized() const {
  return (m_vtkFboRenderer != nullptr);
}

void View::setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine) {
  m_processingEngine = std::shared_ptr<ProcessingEngine>(processingEngine);
}


// Model releated functions

bool View::isModelSelected() const {
  return m_vtkFboRenderer->isModelSelected();
}

double View::getSelectedModelPositionX() const {
  return m_vtkFboRenderer->getSelectedModelPositionX();
}

double View::getSelectedModelPositionY() const {
  return m_vtkFboRenderer->getSelectedModelPositionY();
}


void View::selectModel(const int screenX, const int screenY) {
  m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(screenX, screenY), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  m_lastMouseLeftButton->ignore();

  update();
}

void View::resetModelSelection() {
  m_lastMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  m_lastMouseLeftButton->ignore();

  update();
}

void View::addModelFromFile(const QUrl &modelPath) {
  qDebug() << "QVTKFramebufferObjectItem::addModelFromFile";

  CommandModelAdd *command = new CommandModelAdd(m_vtkFboRenderer, m_processingEngine, modelPath);

  connect(command, &CommandModelAdd::ready, this, &View::update);
  connect(command, &CommandModelAdd::done, this, &View::addModelFromFileDone);

  command->start();

  this->addCommand(command);
}

void View::translateModel(CommandModelTranslate::TranslateParams_t & translateData, const bool inTransition) {
  if (translateData.model == nullptr) {
    // If no model selected yet, try to select one
    translateData.model = m_vtkFboRenderer->getSelectedModel();

    if (translateData.model == nullptr) {
      return;
    }
  }

  this->addCommand(new CommandModelTranslate(m_vtkFboRenderer, translateData, inTransition));
}


void View::addCommand(CommandModel *command) {
  m_commandsQueueMutex.lock();
  m_commandsQueue.push(command);
  m_commandsQueueMutex.unlock();

  update();
}


// Camera related functions

void View::wheelEvent(QWheelEvent *e) {
  m_lastMouseWheel = std::make_shared<QWheelEvent>(*e);
  m_lastMouseWheel->ignore();
  e->accept();
  update();
}

void View::mousePressEvent(QMouseEvent *e) {
  if (e->buttons() & Qt::RightButton) {
    m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
    m_lastMouseButton->ignore();
    e->accept();
    update();
  }
}

void View::mouseReleaseEvent(QMouseEvent *e) {
  m_lastMouseButton = std::make_shared<QMouseEvent>(*e);
  m_lastMouseButton->ignore();
  e->accept();
  update();
}

void View::mouseMoveEvent(QMouseEvent *e) {
  if (e->buttons() & Qt::RightButton) {
    *m_lastMouseMove = *e;
    m_lastMouseMove->ignore();
    e->accept();
    update();
  }
}


QMouseEvent *View::getLastMouseLeftButton() {
  return m_lastMouseLeftButton.get();
}

QMouseEvent *View::getLastMouseButton() {
  return m_lastMouseButton.get();
}

QMouseEvent *View::getLastMoveEvent() {
  return m_lastMouseMove.get();
}

QWheelEvent *View::getLastWheelEvent() {
  return m_lastMouseWheel.get();
}


void View::resetCamera() {
  m_vtkFboRenderer->resetCamera();
  update();
}

int View::getModelsRepresentation() const {
  return m_modelsRepresentationOption;
}

double View::getModelsOpacity() const {
  return m_modelsOpacity;
}

bool View::getGourauInterpolation() const {
  return m_gouraudInterpolation;
}

int View::getModelColorR() const {
  return m_modelColorR;
}

int View::getModelColorG() const {
  return m_modelColorG;
}

int View::getModelColorB() const {
  return m_modelColorB;
}

void View::setModelsRepresentation(const int representationOption) {
  if (m_modelsRepresentationOption != representationOption) {
    m_modelsRepresentationOption = representationOption;
    update();
  }
}

void View::setModelsOpacity(const double opacity) {
  if (m_modelsOpacity != opacity) {
    m_modelsOpacity = opacity;
    update();
  }
}

void View::setGouraudInterpolation(const bool gouraudInterpolation) {
  if (m_gouraudInterpolation != gouraudInterpolation) {
    m_gouraudInterpolation = gouraudInterpolation;
    update();
  }
}

void View::setModelColorR(const int colorR) {
  if (m_modelColorR != colorR) {
    m_modelColorR = colorR;
    update();
  }
}

void View::setModelColorG(const int colorG) {
  if (m_modelColorG != colorG) {
    m_modelColorG = colorG;
    update();
  }
}

void View::setModelColorB(const int colorB) {
  if (m_modelColorB != colorB) {
    m_modelColorB = colorB;
    update();
  }
}

CommandModel *View::getCommandsQueueFront() const {
  return m_commandsQueue.front();
}

void View::commandsQueuePop() {
  m_commandsQueue.pop();
}

bool View::isCommandsQueueEmpty() const {
  return m_commandsQueue.empty();
}

void View::lockCommandsQueueMutex() {
  m_commandsQueueMutex.lock();
}

void View::unlockCommandsQueueMutex() {
  m_commandsQueueMutex.unlock();
}


