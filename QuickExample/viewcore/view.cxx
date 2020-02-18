#include "CommandModel.h"
#include "CommandModelAdd.h"
#include "Model.h"
#include "ProcessingEngine.h"
#include <viewcore/view.hpp>
#include <viewcore/renderer.hpp>


QVTKFramebufferObjectItem::QVTKFramebufferObjectItem() {
  this->m_mouse.evMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  this->m_mouse.evMouseButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  this->m_mouse.evMouseMove = std::make_shared<QMouseEvent>(QEvent::None, QPointF(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  this->m_mouse.evMouseWheel = std::make_shared<QWheelEvent>(QPointF(0,0), 0, Qt::NoButton, Qt::NoModifier, Qt::Vertical);

  this->setMirrorVertically(true); // QtQuick and OpenGL have opposite Y-Axis directions

  setAcceptedMouseButtons(Qt::RightButton);
}


QQuickFramebufferObject::Renderer *QVTKFramebufferObjectItem::createRenderer() const {
  return new QVTKFramebufferObjectRenderer();
}

void QVTKFramebufferObjectItem::setVtkFboRenderer(QVTKFramebufferObjectRenderer* renderer) {
  qDebug() << "QVTKFramebufferObjectItem::setVtkFboRenderer";

  m_pRenderer = renderer;

  connect(m_pRenderer, &QVTKFramebufferObjectRenderer::isModelSelectedChanged, this, &QVTKFramebufferObjectItem::isModelSelectedChanged);
  connect(m_pRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionXChanged, this, &QVTKFramebufferObjectItem::selectedModelPositionXChanged);
  connect(m_pRenderer, &QVTKFramebufferObjectRenderer::selectedModelPositionYChanged, this, &QVTKFramebufferObjectItem::selectedModelPositionYChanged);

  m_pRenderer->setProcessingEngine(m_processingEngine);
}

bool QVTKFramebufferObjectItem::isInitialized() const {
  return (m_pRenderer != nullptr);
}

void QVTKFramebufferObjectItem::setProcessingEngine(const std::shared_ptr<ProcessingEngine> processingEngine) {
  m_processingEngine = std::shared_ptr<ProcessingEngine>(processingEngine);
}


// Model releated functions

bool QVTKFramebufferObjectItem::isModelSelected() const {
  return m_pRenderer->isModelSelected();
}

double QVTKFramebufferObjectItem::getSelectedModelPositionX() const {
  return m_pRenderer->getSelectedModelPositionX();
}

double QVTKFramebufferObjectItem::getSelectedModelPositionY() const {
  return m_pRenderer->getSelectedModelPositionY();
}


void QVTKFramebufferObjectItem::selectModel(const int screenX, const int screenY) {
  this->m_mouse.evMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(screenX, screenY), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  this->m_mouse.evMouseLeftButton->ignore();

  update();
}

void QVTKFramebufferObjectItem::resetModelSelection() {
  this->m_mouse.evMouseLeftButton = std::make_shared<QMouseEvent>(QEvent::None, QPointF(-1, -1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  this->m_mouse.evMouseLeftButton->ignore();

  update();
}

void QVTKFramebufferObjectItem::addModelFromFile(const QUrl &modelPath) {
  qDebug() << "QVTKFramebufferObjectItem::addModelFromFile";

  CommandModelAdd *command = new CommandModelAdd(m_pRenderer, m_processingEngine, modelPath);

  connect(command, &CommandModelAdd::ready, this, &QVTKFramebufferObjectItem::update);
  connect(command, &CommandModelAdd::done, this, &QVTKFramebufferObjectItem::addModelFromFileDone);

  command->start();

  this->addCommand(command);
}

void QVTKFramebufferObjectItem::translateModel(CommandModelTranslate::TranslateParams_t & translateData, const bool inTransition) {
  if (translateData.model == nullptr) {
    // If no model selected yet, try to select one
    translateData.model = m_pRenderer->getSelectedModel();

    if (translateData.model == nullptr) {
      return;
    }
  }

  this->addCommand(new CommandModelTranslate(m_pRenderer, translateData, inTransition));
}


void QVTKFramebufferObjectItem::addCommand(CommandModel *command) {
  m_commandsQueueMutex.lock();
  m_commandsQueue.push(command);
  m_commandsQueueMutex.unlock();

  update();
}


// Camera related functions

void QVTKFramebufferObjectItem::wheelEvent(QWheelEvent *e) {
  this->m_mouse.evMouseWheel = std::make_shared<QWheelEvent>(*e);
  this->m_mouse.evMouseWheel->ignore();
  e->accept();
  update();
}

void QVTKFramebufferObjectItem::mousePressEvent(QMouseEvent *e) {
  if (e->buttons() & Qt::RightButton) {
    this->m_mouse.evMouseButton = std::make_shared<QMouseEvent>(*e);
    this->m_mouse.evMouseButton->ignore();
    e->accept();
    update();
  }
}

void QVTKFramebufferObjectItem::mouseReleaseEvent(QMouseEvent *e) {
  this->m_mouse.evMouseButton = std::make_shared<QMouseEvent>(*e);
  this->m_mouse.evMouseButton->ignore();
  e->accept();
  update();
}

void QVTKFramebufferObjectItem::mouseMoveEvent(QMouseEvent *e) {
  if (e->buttons() & Qt::RightButton) {
    *this->m_mouse.evMouseMove = *e;
    this->m_mouse.evMouseMove->ignore();
    e->accept();
    update();
  }
}


QMouseEvent *QVTKFramebufferObjectItem::getLastMouseLeftButton() {
  return this->m_mouse.evMouseLeftButton.get();
}

QMouseEvent *QVTKFramebufferObjectItem::getLastMouseButton() {
  return this->m_mouse.evMouseButton.get();
}

QMouseEvent *QVTKFramebufferObjectItem::getLastMoveEvent() {
  return this->m_mouse.evMouseMove.get();
}

QWheelEvent *QVTKFramebufferObjectItem::getLastWheelEvent() {
  return this->m_mouse.evMouseWheel.get();
}


void QVTKFramebufferObjectItem::resetCamera() {
  m_pRenderer->resetCamera();
  update();
}

int QVTKFramebufferObjectItem::getModelsRepresentation() const {
  return m_modelsRepresentationOption;
}

double QVTKFramebufferObjectItem::getModelsOpacity() const {
  return m_modelsOpacity;
}

bool QVTKFramebufferObjectItem::getGourauInterpolation() const {
  return m_gouraudInterpolation;
}

int QVTKFramebufferObjectItem::getModelColorR() const {
  return m_modelColorR;
}

int QVTKFramebufferObjectItem::getModelColorG() const {
  return m_modelColorG;
}

int QVTKFramebufferObjectItem::getModelColorB() const {
  return m_modelColorB;
}

void QVTKFramebufferObjectItem::setModelsRepresentation(const int representationOption) {
  if (m_modelsRepresentationOption != representationOption) {
    m_modelsRepresentationOption = representationOption;
    update();
  }
}

void QVTKFramebufferObjectItem::setModelsOpacity(const double opacity) {
  if (m_modelsOpacity != opacity) {
    m_modelsOpacity = opacity;
    update();
  }
}

void QVTKFramebufferObjectItem::setGouraudInterpolation(const bool gouraudInterpolation) {
  if (m_gouraudInterpolation != gouraudInterpolation) {
    m_gouraudInterpolation = gouraudInterpolation;
    update();
  }
}

void QVTKFramebufferObjectItem::setModelColorR(const int colorR) {
  if (m_modelColorR != colorR) {
    m_modelColorR = colorR;
    update();
  }
}

void QVTKFramebufferObjectItem::setModelColorG(const int colorG) {
  if (m_modelColorG != colorG) {
    m_modelColorG = colorG;
    update();
  }
}

void QVTKFramebufferObjectItem::setModelColorB(const int colorB) {
  if (m_modelColorB != colorB) {
    m_modelColorB = colorB;
    update();
  }
}

CommandModel *QVTKFramebufferObjectItem::getCommandsQueueFront() const {
  return m_commandsQueue.front();
}

void QVTKFramebufferObjectItem::commandsQueuePop() {
  m_commandsQueue.pop();
}

bool QVTKFramebufferObjectItem::isCommandsQueueEmpty() const {
  return m_commandsQueue.empty();
}

void QVTKFramebufferObjectItem::lockCommandsQueueMutex() {
  m_commandsQueueMutex.lock();
}

void QVTKFramebufferObjectItem::unlockCommandsQueueMutex() {
  m_commandsQueueMutex.unlock();
}


