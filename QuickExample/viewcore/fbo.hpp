#pragma once

#include <QtQuick/QQuickFramebufferObject>

class QVTKFramebufferObjectRenderer;

class QVTKFramebufferObject : public QQuickFramebufferObject {
  Q_OBJECT

 public:
  QVTKFramebufferObject();

  Renderer *createRenderer() const Q_DECL_OVERRIDE;

 Q_SIGNALS:
  void rendererInitialized();

 private:
  QVTKFramebufferObjectRenderer *m_pVTKFboRenderer = nullptr;
};
