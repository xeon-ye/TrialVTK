import sys
import numpy as np

import vtk

from qtpy import (QtWidgets, QtGui, uic)
from qtpy.QtCore import (qDebug, QObject, Qt)

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

    # Add a layout to let the vtk panel grow/shrink with window resize
    self.vtk_layout = QtWidgets.QHBoxLayout()
    self.vtk_layout.addWidget(self.vtk_widget)
    self.vtk_layout.setContentsMargins(0,0,0,0)
    self.vtk_panel.setLayout(self.vtk_layout)
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
