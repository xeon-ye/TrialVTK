#pragma once

#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>

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
};
