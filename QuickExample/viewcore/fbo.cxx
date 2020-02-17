/**
 * @file   fbo.cxx
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Feb 17 23:23:35 2020
 *
 * @brief
 *
 * Copyright 2020 Jens Munk Hansen
 *
 */
#include <viewcore/fbo.hpp>

#include <memory>

class EventHandler {
 private:
  std::shared_ptr<QMouseEvent> m_lastMouseLeftButton;
  std::shared_ptr<QMouseEvent> m_lastMouseButton;
  std::shared_ptr<QMouseEvent> m_lastMouseMove;
  std::shared_ptr<QWheelEvent> m_lastMouseWheel;
  EventHandler() {
    m_lastMouseLeftButton =
        std::make_shared<QMouseEvent>(QEvent::None, QPointF(0, 0),
                                      Qt::NoButton, Qt::NoButton,
                                      Qt::NoModifier);
    m_lastMouseButton =
        std::make_shared<QMouseEvent>(QEvent::None, QPointF(0, 0),
                                      Qt::NoButton, Qt::NoButton,
                                      Qt::NoModifier);
    m_lastMouseMove =
        std::make_shared<QMouseEvent>(QEvent::None, QPointF(0, 0),
                                      Qt::NoButton, Qt::NoButton,
                                      Qt::NoModifier);
    m_lastMouseWheel =
        std::make_shared<QWheelEvent>(QPointF(0, 0), 0,
                                      Qt::NoButton, Qt::NoModifier,
                                      Qt::Vertical);
  }

 public:
  //  template <typename EventType>
  //  void HandleEvent(EventType* e);
};

QVTKFramebufferObject::QVTKFramebufferObject() : m_pVTKFboRenderer(nullptr) {
  this->setMirrorVertically(true);  // QtQuick and OpenGL
                                    // have opposite Y-Axis directions
  setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);
}
