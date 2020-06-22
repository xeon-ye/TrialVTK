import os
import vtk
import numpy as np

from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtk.util.colors import red, blue, black, yellow, green
from qtpy.QtCore import (qDebug)

from qtpy import (QtWidgets)

from vtkUtils import AxesToTransform

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
    #self.worldScale:float = 0.5
    #self.worldScale:float = 0.025
    self.brighter25:bool = True
    self.opacity:float = 0.35

    self.refplanes = []
    self.planeWidgets = []
    self.contours = []
    self.userAttempts = []
    self.vessels = None
    self.liver = None

    self.initScene()
    self.initLiver()
    self.initVessels() # Must be before planeWidgets

    self.nReferences = 2
    self.lastPositions = dict({'origin' : self.nReferences*[None],
                               'normal' : self.nReferences*[None],
                               'axis1'  : self.nReferences*[None],
                               'reset'  : self.nReferences*[None]})

    self.lastIndex = None
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
    index = self.lastIndex
    if key == 'c':
      print('Reset')
      planeWidget = self.planeWidgets[index]

      self.userAttempts[index].SetUserTransform(None)
      self.userAttempts[index].Modified()

      storeOrigin = self.lastPositions['reset'][0]
      storePoint1 = self.lastPositions['reset'][1]
      storePoint2 = self.lastPositions['reset'][2]

      # Reset planeWidget
      planeWidget.SetOrigin(storeOrigin)
      planeWidget.SetPoint1(storePoint1)
      planeWidget.SetPoint2(storePoint2)

      lastNormal = planeWidget.GetNormal()
      lastAxis1 = vtk.vtkVector3d()
      vtk.vtkMath.Subtract(planeWidget.GetPoint1(),
                           planeWidget.GetOrigin(),
                           lastAxis1)
      lastOrigin = planeWidget.GetCenter()
      self.lastPositions['origin'][index] = lastOrigin
      self.lastPositions['normal'][index] = lastNormal
      self.lastPositions['axis1'][index] = lastAxis1

      testActor = None
      if testActor is not None:
        self.renderer.RemoveActor(testActor)
        testActor = None

    elif key == 's':
      print('Registration')
      # ============ run ICP ==============
      icp = vtk.vtkIterativeClosestPointTransform()

      # Transform contours
      tfpdf0 = vtk.vtkTransformPolyDataFilter()
      tfpdf0.SetInputData(oldContours)
      tfpdf0.SetTransform(self.userAttempt[index].GetUserTransform())
      tfpdf0.Update()
      wrongContours = tfpdf0.GetOutput()
      icp.SetSource(wrongContours)
      icp.SetTarget(vesselPolyData)
      icp.GetLandmarkTransform().SetModeToRigidBody()
      icp.DebugOn()
      icp.SetMaximumNumberOfIterations(10)
      icp.StartByMatchingCentroidsOff()
      #icp.SetMeanDistanceModeToRMS()
      icp.SetMeanDistanceModeToAbsoluteValue()

      #icp.StartByMatchingCentroidsOn()
      icp.Modified()
      icp.Update()
      icpTransformFilter = vtk.vtkTransformPolyDataFilter()
      icpTransformFilter.SetInputData(wrongContours)
      icpTransformFilter.SetTransform(icp)
      icpTransformFilter.Update()

      correctedContours = icpTransformFilter.GetOutput()

      tubes = vtk.vtkTubeFilter()
      tubes.SetInputData(correctedContours)
      tubes.CappingOn()
      tubes.SidesShareVerticesOff()
      tubes.SetNumberOfSides(12)
      tubes.SetRadius(1.0)

      edgeMapper = vtk.vtkPolyDataMapper()
      edgeMapper.ScalarVisibilityOff()
      edgeMapper.SetInputConnection(tubes.GetOutputPort())

      if testActor is not None:
        ren.RemoveActor(testActor)
      testActor = vtk.vtkActor()
      testActor.SetMapper(edgeMapper)
      prop = testActor.GetProperty()
      prop.SetColor(colors.GetColor3d("Yellow"))
      ren.AddActor(testActor)

      # Reset afterwards
      self.userAttempt[index].SetUserTransform(None)
      self.userAttempt[index].Modified()

      # Reset planeWidget
      planeWidgets[index].SetOrigin(storeOrigin)
      planeWidgets[index].SetPoint1(storePoint1)
      planeWidgets[index].SetPoint2(storePoint2)

      lastNormal = planeWidgets[index].GetNormal()
      lastAxis1 = vtk.vtkVector3d()
      vtk.vtkMath.Subtract(planeWidgets[index].GetPoint1(),
                           planeWidgets[index].GetOrigin(),
                           lastAxis1)
      lastOrigin = planeWidgets[index].GetCenter()

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
    refplanes.append(np.array([[0.99722,   -0.00658366270883304, 0.0741938205770847, -321.64],
                               [0.07419,   0.17584, -0.98162, -227.46],
                               [-0.0065837, 0.98440,  0.17584, -563.67],
                               [0,  0,      0,      1]]))
    centers = []
    centers.append(np.r_[-75.696,        -149.42,    -231.76])

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

    # Should transformation also be scaled??
    for i in range(3):
      mat.SetElement(i, 3, self.worldScale* mat.GetElement(i,3))

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
    mapper0 = vtk.vtkPolyDataMapper()
    mapper0.SetInputConnection(source.GetOutputPort())

    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper0)
    prop = actor.GetProperty()
    prop.SetColor(blue)
    prop.SetOpacity(0.5)
    self.refplanes.append(actor)
    self.renderer.AddActor(actor)

    # Plane for intersection
    plane = vtk.vtkPlane()
    plane.SetOrigin(source.GetOrigin())
    plane.SetNormal(source.GetNormal())

    cutEdges = vtk.vtkCutter()
    cutEdges.SetInputConnection(self.vesselNormals.GetOutputPort())
    cutEdges.SetCutFunction(plane)
    cutEdges.GenerateCutScalarsOff() # Was on
    cutEdges.SetValue(0, 0.5)
    cutEdges.Update()

    cutStrips = vtk.vtkStripper()
    cutStrips.SetInputConnection(cutEdges.GetOutputPort())
    cutStrips.Update()

    tubes = vtk.vtkTubeFilter()
    tubes.SetInputConnection(cutStrips.GetOutputPort()) # works
    tubes.CappingOn()
    tubes.SidesShareVerticesOff()
    tubes.SetNumberOfSides(12)
    tubes.SetRadius(self.worldScale*2.0)

    edgeMapper = vtk.vtkPolyDataMapper()
    edgeMapper.ScalarVisibilityOff()
    edgeMapper.SetInputConnection(tubes.GetOutputPort())

    # Clipping planes
    planes = vtk.vtkPlaneCollection()

    axis1 = np.array(source.GetPoint1()) - np.array(source.GetOrigin())
    axis2 = np.array(source.GetPoint2()) - np.array(source.GetOrigin())

    # Okay
    plane1 = vtk.vtkPlane()
    plane1.SetOrigin(source.GetOrigin())
    plane1.SetNormal(axis2)

    plane2 = vtk.vtkPlane()
    plane2.SetOrigin(source.GetOrigin())
    plane2.SetNormal(axis1)

    tmp = axis2 + np.array(source.GetPoint1())

    plane3 = vtk.vtkPlane()
    plane3.SetOrigin(tmp)
    plane3.SetNormal(-axis2)

    plane4 = vtk.vtkPlane()
    plane4.SetOrigin(tmp)
    plane4.SetNormal(-axis1)

    planes.AddItem(plane1)
    planes.AddItem(plane2)
    planes.AddItem(plane3)
    planes.AddItem(plane4)

    edgeMapper.SetClippingPlanes(planes)
    actor = vtk.vtkActor()
    actor.SetMapper(edgeMapper)
    prop = actor.GetProperty()
    prop.SetColor(green)
    prop.SetLineWidth(3)
    self.contours.append(actor)
    self.renderer.AddActor(actor)

    # Plane widget
    planeWidget = vtk.vtkPlaneWidget()
    planeWidget.SetInteractor(self.interactor)
    planeWidget.SetOrigin(source.GetOrigin())
    planeWidget.SetPoint1(source.GetPoint1())
    planeWidget.SetPoint2(source.GetPoint2())
    prop = planeWidget.GetHandleProperty()
    prop.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    prop = planeWidget.GetPlaneProperty()
    prop.SetColor(QLiverViewer.colors.GetColor3d("Red"))

    self.lastPositions['reset'][index] = (planeWidget.GetOrigin(),
                                          planeWidget.GetPoint1(),
                                          planeWidget.GetPoint2())
    planeWidget.SetEnabled(1)
    planeWidget.AddObserver(vtk.vtkCommand.EndInteractionEvent, self.widgetMoved, 1.0)
    self.planeWidgets.append(planeWidget)

    attempt = vtk.vtkActor()
    oldContours = cutStrips.GetOutput()
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputData(oldContours)
    attempt.SetMapper(mapper)
    attempt.GetProperty().SetColor(red)
    self.userAttempts.append(attempt)
    self.renderer.AddActor(attempt)

    lastNormal = planeWidget.GetNormal()
    lastAxis1 = vtk.vtkVector3d()
    vtk.vtkMath.Subtract(planeWidget.GetPoint1(),
                         planeWidget.GetOrigin(),
                         lastAxis1)
    lastOrigin = planeWidget.GetCenter()
    self.lastPositions['origin'][index] = lastOrigin
    self.lastPositions['normal'][index] = lastNormal
    self.lastPositions['axis1'][index] = lastAxis1



  def widgetMoved(self, obj, ev):
    if (obj in self.planeWidgets):
      index = self.planeWidgets.index(obj)
      self.lastIndex = index

      normal0 = self.lastPositions['normal'][index]
      first0  = self.lastPositions['axis1'][index]
      origin0 = self.lastPositions['origin'][index]

      normal1 = np.array(obj.GetNormal())
      first1 = np.array(obj.GetPoint1()) - np.array(obj.GetOrigin())
      origin1 = obj.GetCenter()

      trans = AxesToTransform(normal0, first0, origin0,
                              normal1, first1, origin1)

      if self.userAttempts[index].GetUserTransform() is not None:
        self.userAttempts[index].GetUserTransform().Concatenate(trans)
      else:
        transform = vtk.vtkTransform()
        transform.SetMatrix(trans)
        transform.PostMultiply()
        self.userAttempts[index].SetUserTransform(transform)

      self.lastPositions['origin'][index] = obj.GetCenter()

      lastAxis1 = [first1[0], first1[1], first1[2]]
      lastNormal = (normal1[0], normal1[1], normal1[2])

      self.lastPositions['axis1'][index] = lastAxis1
      self.lastPositions['normal'][index] = lastNormal

      self.userAttempts[index].Modified()

    #print(ev)

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
    self.vesselNormals = vtk.vtkPolyDataNormals()
    self.vesselNormals.SetInputData(self.vesselPolyData)

    # mapper
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(self.vesselNormals.GetOutputPort())

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

