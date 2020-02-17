/**
 * @file   renderer.cxx
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Feb 17 21:19:41 2020
 *
 * @brief
 *
 * Copyright 2020 Jens Munk Hansen
 *
 */

#include <viewcore/renderer.hpp>
// #include <viewcore/eventhandler.hpp>

#include <QSurfaceFormat>

#include <vtkInteractorStyleTrackballCamera.h>

#include <QVTKOpenGLNativeWidget.h>

RenderHelper::RenderHelper() {

}

QVTKFramebufferObjectRenderer::QVTKFramebufferObjectRenderer() :
    m_pFBO(nullptr), m_bFirstRender(true) {
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

  // Window and renderer
  m_vtkRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_vtkRenderWindow->AddRenderer(m_renderer);

  // Interactor
  m_vtkRenderWindowInteractor =
      vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();

  // Disable message loop for interactor
  m_vtkRenderWindowInteractor->EnableRenderOff();
  m_vtkRenderWindow->SetInteractor(m_vtkRenderWindowInteractor);

  // Initialize the OpenGL context for the renderer
  m_vtkRenderWindow->OpenGLInitContext();

  // Interactor Style
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
      vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  style->SetDefaultRenderer(m_renderer);
  style->SetMotionFactor(10.0);
  m_vtkRenderWindowInteractor->SetInteractorStyle(style);

  // Picker
  m_picker = vtkSmartPointer<vtkCellPicker>::New();
  m_picker->SetTolerance(0.0);

  update();
}

void QVTKFramebufferObjectRenderer::openGLInitState() {
  m_vtkRenderWindow->OpenGLInitState();
  m_vtkRenderWindow->MakeCurrent();
  //QOpenGLFunctions::initializeOpenGLFunctions();
  //QOpenGLFunctions::glUseProgram(0);
}

void QVTKFramebufferObjectRenderer::synchronize(QQuickFramebufferObject *pFBO) {
  // For the first synchronize
  if (!m_pFBO) {
    m_pFBO = static_cast<QVTKFramebufferObjectItem *>(item);
  }

  if (!m_pFBO->isInitialized()) {
    m_pFBO->setVtkFboRenderer(this);

    emit m_pFBO->rendererInitialized();
  }

  int *rendererSize = m_vtkRenderWindow->GetSize();

  if (m_pFBO->width() != rendererSize[0] ||
      m_pFBO->height() != rendererSize[1]) {
    m_vtkRenderWindow->SetSize(m_pFBO->width(), m_pFBO->height());
  }

  return;
}

void QVTKFramebufferObjectRenderer::render() {
  m_vtkRenderWindow->PushState();
  this->openGLInitState();
  // Start rendering process for this frame
  m_vtkRenderWindow->Start();

  if (m_firstRender) {
    // this->initScene();
    m_firstRender = false;
  }
  return;
}

QOpenGLFramebufferObject*
QVTKFramebufferObjectRenderer::createFramebufferObject(const QSize& size) {
  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::Depth);

#ifdef Q_OS_MAC
  QSize macSize = QSize(size.width() / 2, size.height() / 2);
  QOpenGLFramebufferObject* pFBO =
      new QOpenGLFramebufferObject(macSize, format);
#else
  QOpenGLFramebufferObject* pFBO =
      new QOpenGLFramebufferObject(size, format);
#endif

  m_vtkRenderWindow->SetBackLeftBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetFrontLeftBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetBackBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetFrontBuffer(GL_COLOR_ATTACHMENT0);
  m_vtkRenderWindow->SetSize(pFBO->size().width(),
                             pFBO->size().height());
  m_vtkRenderWindow->SetOffScreenRendering(true);
  m_vtkRenderWindow->Modified();

  return pFBO;
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
