import os
import vtk
import numpy as np

from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtk.util.colors import red, blue, black, yellow, green
from qtpy.QtCore import (qDebug)

from qtpy import (QtWidgets)

def hexCol(s):
  if isinstance(s,str):
    if "#" in s:  # hex to rgb
      h = s.lstrip("#")
      rgb255 = list(int(h[i : i + 2], 16) for i in (0, 2, 4))
      rgbh = np.array(rgb255) / 255.0
      return tuple(rgbh)

class QLiverViewer(QtWidgets.QFrame):
  colors = vtk.vtkNamedColors()

  def __init__(self, parent):
    super(QLiverViewer,self).__init__(parent)

    # Make the actual QtWidget a child so that it can be re_parented
    self.interactor = QVTKRenderWindowInteractor(self)
    self.layout = QtWidgets.QHBoxLayout()
    self.layout.addWidget(self.interactor)
    self.layout.setContentsMargins(0,0,0,0)
    self.setLayout(self.layout)

    self.worldScale = 1.0
    self.worldScale = 0.025
    self.brighter25 = True
    self.opacity = 0.35

    self.refplanes = []

    self.platformWidth:float = 200.0
    self.platformDepth:float = 200.0
    self.platformThickness:float = 2.0
    self.gridBottomHeight:float = 0.15
    self.gridSize:np.uint16 = 10

    self.initScene()
    self.initLiver()
    self.initPlaneWidgets()
    #self.initPlatform()
    #self.initAxes()

    self.resetCamera()

    self.style = vtk.vtkInteractorStyleTrackballCamera()
    self.style.SetDefaultRenderer(self.renderer)
    self.interactor.SetInteractorStyle(self.style)

  def scale(self, polyData):
    if self.worldScale == 1.0:
      return polyData
    else:
      transform = vtk.vtkTransform()
      transform.Scale(self.worldScale,
                      self.worldScale,
                      self.worldScale)
      transformFilter = vtk.vtkTransformPolyDataFilter()
      transformFilter.SetInputData(polyData)
      transformFilter.SetTransform(transform)
      transformFilter.Update()
      return transformFilter.GetOutput()

  def getReferencePosition(self, index):
    refplanes = []
    refplanes.append(np.array([[0.9972221078443415,   -0.00658366270883304, 0.0741938205770847, -321.63981996062756],
                               [0.07419382057708453,   0.1758409118814136, -0.9816186890523674, -227.4617768729491],
                               [-0.006583662708833091, 0.9843965812080258,  0.1758409118814139, -563.6745122183239],
                               [0,  0,      0,      1]]))
    centers = []
    centers.append(np.r_[-75.69560243429154,        -149.41579842495196,    -231.76170945617494])

    refplanes.append(np.array([[0.9824507428729312,   -0.028608856565971154, 0.1843151408713164, -221.425151769367],
                               [0.18431514087131629,   0.3004711475787132,  -0.935812491003576,  -325.6553959586223],
                               [-0.028608856565971223, 0.9533617481306448,   0.3004711475787133, -547.1574253306663],
                               [0,  0,      0,      1]]))

    centers.append(np.r_[-31.317285034663634,       -174.62449255285645,    -193.39018826551072])
    return centers[index], refplanes[index]
  def resetCamera(self):
    qDebug('resetCamera')
    self.renderer.ResetCamera()

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

  def initPlaneWidgets(self):
    qDebug('initPlaneWidgets()')
    center, htrans = self.getReferencePosition(0)

    hw = 50 # mm
    shw = self.worldScale * hw
    source = vtk.vtkPlaneSource()
    source.SetOrigin(0,  0, 0)
    source.SetPoint1(shw, 0, 0)
    source.SetPoint2(0, shw, 0)

    transform = vtk.vtkTransform()
    mat = vtk.vtkMatrix4x4()
    for i in range(4):
      for j in range(4):
        mat.SetElement(i, j, htrans[i,j])

    transform.SetMatrix(mat)
    transform.Update()

    origin = source.GetOrigin()
    origin = transform.TransformPoint(origin)
    source.SetOrigin(origin)

    p1 = source.GetPoint1()
    p1 = transform.TransformPoint(p1)
    source.SetPoint1(p1)

    p2 = source.GetPoint2()
    p2 = transform.TransformPoint(p2)
    source.SetPoint2(p2)

    source.Update()

    source.SetCenter(self.worldScale * center)
    source.Update()

    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())

    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    prop = actor.GetProperty()
    prop.SetColor(blue)
    prop.SetOpacity(0.5)
    self.refplanes.append(actor)
    self.renderer.AddActor(actor)

    self.planeWidget = vtk.vtkPlaneWidget()
    self.planeWidget.SetInteractor(self.interactor)
    self.planeWidget.SetOrigin(source.GetOrigin())
    self.planeWidget.SetPoint1(source.GetPoint1())
    self.planeWidget.SetPoint2(source.GetPoint2())
    bum = self.planeWidget.GetHandleProperty()
    bum.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    bum = self.planeWidget.GetPlaneProperty()
    bum.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    self.planeWidget.SetEnabled(1)

  def initLiver(self):
    qDebug('initLiver()')
    if os.name == 'nt':
      filename = 'e:/analogic/TrialVTK/data/Abdomen/Liver_3D-interpolation.vtp'
    else:
      filename = '/home/jmh/bkmedical/data/CT/Liver_3D-interpolation.vtp'
    reader = vtk.vtkXMLPolyDataReader()
    reader.SetFileName(filename)
    reader.Update()

    surfNormals = vtk.vtkPolyDataNormals()
    #surfNormals.SetInputConnection(reader.GetOutputPort())
    surfNormals.SetInputData(self.scale(reader.GetOutput()))

    #Create a mapper and actor
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(surfNormals.GetOutputPort()) # was reader
    self.liver = vtk.vtkActor()
    self.liver.SetMapper(mapper)
    prop = self.liver.GetProperty()

    if self.brighter25:
      prop.SetColor(vtk.vtkColor3d(hexCol("#873927")))
    else:
      prop.SetColor(vtk.vtkColor3d(hexCol("#6c2e1f")))
    prop.SetOpacity(self.opacity)
    self.renderer.AddActor(self.liver)

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
