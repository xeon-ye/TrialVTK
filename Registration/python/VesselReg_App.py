import sys
import numpy as np

import vtk

from qtpy import (QtWidgets, QtGui, uic)
from qtpy.QtCore import (qDebug, QObject, Qt, Slot)

from LiverView import QLiverViewer

ui, QMainWindow = uic.loadUiType('VesselReg_App_Qt_VTK.ui')

colors = vtk.vtkNamedColors()

class ViewersApp(QMainWindow, ui):
  def __init__(self):
    # Parent constructor
    super(ViewersApp, self).__init__()
    self.vtk_widget = None
    self.setup()

  def setup(self):
    self.setupUi(self)

    self.vtk_widget = QLiverViewer(self.vtk_panel)
    self.vtk_widget.widgetMoved.connect(self.onWidgetMoved)
    self.vtk_widget.widgetRegistered.connect(self.onWidgetRegistered)

    # Add a layout to let the vtk panel grow/shrink with window resize
    self.vtk_layout = QtWidgets.QHBoxLayout()
    self.vtk_layout.addWidget(self.vtk_widget)
    self.vtk_layout.setContentsMargins(0,0,0,0)
    self.vtk_panel.setLayout(self.vtk_layout)

  @Slot(object, object, object)
  def onWidgetRegistered(self, rot, normal, p):
    print("TODO: Update GUI")

  @Slot(object, object, object)
  def onWidgetMoved(self, rot, normal, p):
    qDebug('onWidgetMoved')
    # Update GUI
    U = rot[1]
    sgn = np.sign(np.dot(U,normal))
    udotn = float("{:.5f}".format(np.abs(np.dot(U,normal))))
    self.leUdotN.setText(str(udotn))
    trans = float("{:.5f}".format(np.sqrt(np.sum(p**2))))
    self.leTransDist.setText(str(trans))
    arg = rot[0]
    arg = np.fmod(arg + 180.0, 360.0) - 180.0
    arg = float("{:.5f}".format(arg*sgn))
    self.leAngle.setText(str(arg))

  def initialize(self):
    self.vtk_widget.start()

if __name__ == '__main__':
  app = QtWidgets.QApplication(["Liver Registration"])
  main_window = ViewersApp()
  main_window.show()
  main_window.initialize()
  app.exec_()


# Local variables: #
# tab-width: 2 #
# python-indent: 2 #
# python-indent-offset: 2 #
# indent-tabs-mode: nil #
# End: #
