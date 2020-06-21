import vtk
import numpy as np

from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from qtpy.QtCore import (qDebug)

from qtpy import (QtWidgets)

class QLiverViewer(QtWidgets.QFrame):
  def __init__(self, parent):
    super(QLiverViewer,self).__init__(parent)

    # Make the actual QtWidget a child so that it can be re_parented
    self.interactor = QVTKRenderWindowInteractor(self)
    self.layout = QtWidgets.QHBoxLayout()
    self.layout.addWidget(self.interactor)
    self.layout.setContentsMargins(0,0,0,0)
    self.setLayout(self.layout)

    self.platformWidth:float = 200.0
    self.platformDepth:float = 200.0
    self.platformThickness:float = 2.0
    self.gridBottomHeight:float = 0.15
    self.gridSize:np.uint16 = 10


    self.initScene()
    self.initPlatform()
    #self.initAxes()
    self.resetCamera()

    self.style = vtk.vtkInteractorStyleTrackballCamera()
    self.style.SetDefaultRenderer(self.renderer)
    self.interactor.SetInteractorStyle(self.style)

  def resetCamera(self):
    qDebug('resetCamera')

  def initScene(self):
    qDebug('initScene()')
    self.renderer =  vtk.vtkOpenGLRenderer()
    self.render_window = self.interactor.GetRenderWindow()
    self.render_window.AddRenderer(self.renderer)

    self.cellPicker = vtk.vtkCellPicker()
    self.cellPicker.SetTolerance(30.0)
    self.interactor.SetPicker(self.cellPicker)

    #* Top background color
    bg_t = np.ones(3)*245.0/255.0

    #* Bottom background color
    bg_b = np.ones(3)*170.0/255.0

    self.renderer.SetBackground(bg_t)
    self.renderer.SetBackground2(bg_b)
    self.renderer.GradientBackgroundOn()

  def initPlatform(self):
    qDebug('initPlatform()')

    #* Platform Model
    platformModelMapper = vtk.vtkPolyDataMapper()

    self.platformModel = vtk.vtkCubeSource()
    platformModelMapper.SetInputConnection(self.platformModel.GetOutputPort())

    self.platformModelActor = vtk.vtkActor()
    self.platformModelActor.SetMapper(platformModelMapper)
    self.platformModelActor.GetProperty().SetColor(1, 1, 1)
    self.platformModelActor.GetProperty().LightingOn()
    self.platformModelActor.GetProperty().SetOpacity(1)
    self.platformModelActor.GetProperty().SetAmbient(0.45)
    self.platformModelActor.GetProperty().SetDiffuse(0.4)

    self.platformModelActor.PickableOff()
    self.renderer.AddActor(self.platformModelActor)

    #* Platform Grid
    self.platformGrid = vtk.vtkPolyData()

    platformGridMapper = vtk.vtkPolyDataMapper()
    platformGridMapper.SetInputData(self.platformGrid)

    self.platformGridActor = vtk.vtkActor()
    self.platformGridActor.SetMapper(platformGridMapper)
    self.platformGridActor.GetProperty().LightingOff()
    self.platformGridActor.GetProperty().SetColor(0.45, 0.45, 0.45)
    self.platformGridActor.GetProperty().SetOpacity(1)
    self.platformGridActor.PickableOff()
    self.renderer.AddActor(self.platformGridActor)
    self.updatePlatform()

  def updatePlatform(self):
    qDebug('updatePlatform()')

    #* Platform Model
    if self.platformModel:
      self.platformModel.SetXLength(self.platformWidth)
      self.platformModel.SetYLength(self.platformDepth)
      self.platformModel.SetZLength(self.platformThickness)
      self.platformModel.SetCenter(0.0, 0.0, -self.platformThickness / 2)

    #* Platform Grid
    gridPoints = vtk.vtkPoints()
    gridCells = vtk.vtkCellArray()

    i = -self.platformWidth / 2
    while i <= self.platformWidth / 2:
      self.createLine(i, -self.platformDepth / 2, self.gridBottomHeight, i, self.platformDepth / 2, self.gridBottomHeight, gridPoints, gridCells)
      i += self.gridSize

    i = -self.platformDepth / 2
    while i <= self.platformDepth / 2:
      self.createLine(-self.platformWidth / 2, i, self.gridBottomHeight, self.platformWidth / 2, i, self.gridBottomHeight, gridPoints, gridCells)
      i += self.gridSize

    self.platformGrid.SetPoints(gridPoints)
    self.platformGrid.SetLines(gridCells)

  def createLine(self, x1:float, y1:float, z1:float, x2:float, y2:float, z2:float, points:vtk.vtkPoints, cells:vtk.vtkCellArray):
    line = vtk.vtkPolyLine()
    line.GetPointIds().SetNumberOfIds(2)

    id_1 = points.InsertNextPoint(x1, y1, z1) # vtkIdType
    id_2 = points.InsertNextPoint(x2, y2, z2) # vtkIdType

    line.GetPointIds().SetId(0, id_1)
    line.GetPointIds().SetId(1, id_2)
    cells.InsertNextCell(line)

  def start(self):
    self.interactor.Initialize()
    # If a big Qt application, call app.exec instead of having two GUI threads
    self.interactor.Start()

  def removeActor(self, actor):
    self.interactor.Disable()
    self.renderer.RemoveActor(actor)
    self.interactor.Enable()
    self.render_window.Render()

  def addActor(self, actor):
    self.interactor.Disable()
    self.renderer.AddActor(actor)
    self.interactor.Enable()
    self.render_window.Render()
