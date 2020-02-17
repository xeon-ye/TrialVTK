#include <viewcore/fbo.hpp>

class

QVTKFramebufferObject::QVTKFramebufferObject() : m_pVTKFboRenderer(nullptr) {
  this->setMirrorVertically(true);  // QtQuick and OpenGL
                                    // have opposite Y-Axis directions
  setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);


}
