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

def onKeyPressed(obj, ev):
  key = obj.GetKeySym()
  print(key, 'was pressed')

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

    self.imgWidth:float = 50.0 # mm
    self.worldScale:float = 1.0
    self.worldScale:float = 0.025
    self.brighter25:bool = True
    self.opacity:float = 0.35

    self.refplanes = []
    self.planeWidgets = []
    self.vessels = None
    self.liver = None

    self.initScene()
    self.initLiver()
    self.initVessels()
    self.initPlaneWidgets(0)
    self.initPlaneWidgets(1)
    #self.initAxes()

    self.resetCamera()

    self.style = vtk.vtkInteractorStyleTrackballCamera()
    self.style.SetDefaultRenderer(self.renderer)
    self.interactor.SetInteractorStyle(self.style)

    self.interactor.AddObserver('KeyPressEvent', self.bum, 1.0)

  def bum(self, obj, ev):
    key = obj.GetKeySym()
    print(key, 'was pressed')

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

  def initPlaneWidgets(self, index):
    qDebug('initPlaneWidgets()')
    center, htrans = self.getReferencePosition(index)

    hw = self.imgWidth
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

    # mapper
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

    planeWidget = vtk.vtkPlaneWidget()
    planeWidget.SetInteractor(self.interactor)
    planeWidget.SetOrigin(source.GetOrigin())
    planeWidget.SetPoint1(source.GetPoint1())
    planeWidget.SetPoint2(source.GetPoint2())
    bum = planeWidget.GetHandleProperty()
    bum.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    bum = planeWidget.GetPlaneProperty()
    bum.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    planeWidget.SetEnabled(1)
    planeWidget.AddObserver(vtk.vtkCommand.EndInteractionEvent, self.widgetMoved, 1.0)
    self.planeWidgets.append(planeWidget)

  def widgetMoved(self, obj, ev):
    print(obj)
    print(ev)
  def initVessels(self):
    qDebug('initVessels()')
    if os.name == 'nt':
      filename = 'e:/analogic/TrialVTK/data/Abdomen/A.vtp'
    else:
      filename = '/home/jmh/bkmedical/data/CT/Connected.vtp'
    # read data
    reader = vtk.vtkXMLPolyDataReader()
    reader.SetFileName(filename)
    reader.Update()

    self.vesselPolyData = self.scale(reader.GetOutput())

    # compute normals
    vesselNormals = vtk.vtkPolyDataNormals()
    vesselNormals.SetInputData(self.vesselPolyData)

    # mapper
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(vesselNormals.GetOutputPort())

    # actor for vessels
    self.vessels = vtk.vtkActor()
    self.vessels.SetMapper(mapper)
    prop = self.vessels.GetProperty()

    if self.brighter25:
      prop.SetColor(vtk.vtkColor3d(hexCol("#517487"))) # 25% lighter
    else:
      prop.SetColor(vtk.vtkColor3d(hexCol("#415d6c")))
    # assign actor to the renderer
    self.renderer.AddActor(self.vessels)

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



  def start(self):
    self.interactor.Initialize()
    # If a big Qt application, call app.exec instead of having two GUI threads
    self.interactor.Start()

