#pragma once

#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>

//#include <vtkActor.h>
//#include <vtkCellPicker.h>
//#include <vtkCubeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkObject.h>
//#include <vtkPoints.h>
//#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCellPicker.h>

class RenderHelper;

class QVTKFramebufferObject;

class QVTKFramebufferObjectRenderer :
  public QObject, public QQuickFramebufferObject::Renderer {
  Q_OBJECT
 public:
  QVTKFramebufferObjectRenderer();
  void synchronize(QQuickFramebufferObject *item) Q_DECL_OVERRIDE;
  void render() Q_DECL_OVERRIDE;
  QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) Q_DECL_OVERRIDE;
  void openGLInitState();
 Q_SIGNALS:

 private:
  QVTKFramebufferObject *m_pFBO = nullptr;
  RenderHelper* m_pRenderHelper = nullptr;
  bool m_bFirstRender;

  vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_vtkRenderWindow;
  vtkSmartPointer<vtkRenderer> m_renderer;
  vtkSmartPointer<vtkGenericRenderWindowInteractor> m_vtkRenderWindowInteractor;
  vtkSmartPointer<vtkCellPicker> m_picker;

  friend class QVTKFramebufferObject;
};

class RenderHelper {
  RenderHelper();
};
