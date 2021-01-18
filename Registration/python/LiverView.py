# TODO: Make scaling work. Get distance
import sys
import os
import vtk
import numpy as np

from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor
from vtk.util.colors import red, blue, black, yellow, green
from qtpy.QtCore import (qDebug, Signal, Slot)

from qtpy import (QtWidgets)

from vtkUtils import AxesToTransform

IOUSFAN = False

filedir = os.path.dirname(os.path.realpath(__file__))

def hexCol(s):
  if isinstance(s,str):
    if "#" in s:  # hex to rgb
      h = s.lstrip("#")
      rgb255 = list(int(h[i : i + 2], 16) for i in (0, 2, 4))
      rgbh = np.array(rgb255) / 255.0
      return tuple(rgbh)

class QLiverViewer(QtWidgets.QFrame):
  colors = vtk.vtkNamedColors()
  widgetMoved = Signal(object, object, object)#float)

  widgetRegistered = Signal(object, object, object)

  def __init__(self, parent):
    super(QLiverViewer,self).__init__(parent)

    # Make the actual QtWidget a child so that it can be re_parented
    self.interactor = QVTKRenderWindowInteractor(self)
    self.layout = QtWidgets.QHBoxLayout()
    self.layout.addWidget(self.interactor)
    self.layout.setContentsMargins(0,0,0,0)
    self.setLayout(self.layout)

    self.imgWidth:float = 50.0 # mm
    self.worldScale:float = 0.05
    self.worldScale:float = 1.0
    self.brighter25:bool = True
    self.opacity:float = 0.35

    self.nReferences = 2

    self.refplanes = []    # Displayed reference planes
    self.planeWidgets = [] # Widgets for manipulation
    self.contours = []     # Contours clipped to reference plane
    self.fullcontours = [] # Full contour data
    self.planeSources = []
    self.userAttempts = []
    self.contourResults = self.nReferences * [None]
    self.vessels = None
    self.liver = None

    self.initScene()
    self.initLiver()
    self.initVessels() # Must be before planeWidgets

    self.lastPositions = dict({'origin' : self.nReferences*[None],
                               'normal' : self.nReferences*[None],
                               'axis1'  : self.nReferences*[None],
                               'reset'  : self.nReferences*[None]})

    self.lastIndex = None
    self.initPlaneWidgets(0)
    if not IOUSFAN:
      self.initPlaneWidgets(1)

    self.resetCamera()

    self.style = vtk.vtkInteractorStyleTrackballCamera()
    self.style.SetDefaultRenderer(self.renderer)
    self.interactor.SetInteractorStyle(self.style)

    self.interactor.AddObserver('KeyPressEvent', self.KeyPress, 1.0)

  def KeyPress(self, obj, ev):
    key = obj.GetKeySym()
    index = self.lastIndex
    if self.lastIndex is None:
      return
    userAttempt = self.userAttempts[index]
    planeWidget = self.planeWidgets[index]
    refActor    = self.refplanes[index]
    resetPositions = self.lastPositions['reset'][index]
    planeSource = self.planeSources[index]
    contourActor = self.contours[index]
    if key == 'c':
      print('Reset')
      userAttempt.SetUserTransform(None)
      userAttempt.Modified()

      # Reset planeWidget
      planeWidget.SetEnabled(0)
      planeWidget.SetOrigin(resetPositions[0])
      planeWidget.SetPoint1(resetPositions[1])
      planeWidget.SetPoint2(resetPositions[2])
      planeWidget.Modified()
      planeWidget.SetEnabled(1)

      lastNormal = planeWidget.GetNormal()
      lastAxis1 = vtk.vtkVector3d()
      vtk.vtkMath.Subtract(planeWidget.GetPoint1(),
                           planeWidget.GetOrigin(),
                           lastAxis1)
      lastOrigin = planeWidget.GetCenter()
      self.lastPositions['origin'][index] = lastOrigin
      self.lastPositions['normal'][index] = lastNormal
      self.lastPositions['axis1'][index] = lastAxis1

      if self.contourResults[index] is not None:
        self.renderer.RemoveActor(self.contourResults[index])
        self.contourResults[index] = None
      self.render_window.Render()
    elif key == 's':
      print('Registration')
      # ============ run ICP ==============
      icp = vtk.vtkIterativeClosestPointTransform()

      # Transform contours
      tfpdf0 = vtk.vtkTransformPolyDataFilter()
      tfpdf0.SetInputData(self.fullcontours[index])
      tfpdf0.SetTransform(userAttempt.GetUserTransform())
      tfpdf0.Update()
      wrongContours = tfpdf0.GetOutput()
      icp.SetSource(wrongContours)
      icp.SetTarget(self.vesselPolyData)
      icp.GetLandmarkTransform().SetModeToRigidBody()
      icp.DebugOn()
      icp.SetMaximumNumberOfIterations(10)
      icp.StartByMatchingCentroidsOff()
      icp.SetCheckMeanDistance(1) # Experiment
      #icp.SetMeanDistanceModeToAbsoluteValue() # Original
      icp.SetMeanDistanceModeToRMS() # Default
      #print(icp.GetLandmarkTransform())
      #sys.stdout.write("Before: ")
      #print(icp.GetMeanDistance())
      from vtkUtils import CloudMeanDist
      dist0 = CloudMeanDist(wrongContours, self.vesselPolyData)
      print(dist0)

      icp.Modified()
      icp.Update()
      #print(icp) # Shows mean distance
      #print(icp.GetLandmarkTransform())

      icpTransformFilter = vtk.vtkTransformPolyDataFilter()
      icpTransformFilter.SetInputData(wrongContours)
      icpTransformFilter.SetTransform(icp)
      icpTransformFilter.Update()

      correctedContours = icpTransformFilter.GetOutput()
      dist1 = CloudMeanDist(correctedContours, self.vesselPolyData)
      print(dist1)
      #sys.stdout.write("After: ")
      #print(icp.GetMeanDistance())

      tubes = vtk.vtkTubeFilter()
      tubes.SetInputData(correctedContours)
      tubes.CappingOn()
      tubes.SidesShareVerticesOff()
      tubes.SetNumberOfSides(12)
      tubes.SetRadius(1.0)

      edgeMapper = vtk.vtkPolyDataMapper()
      edgeMapper.ScalarVisibilityOff()
      edgeMapper.SetInputConnection(tubes.GetOutputPort())

      if self.contourResults[index] is not None:
        self.renderer.RemoveActor(self.contourResults[index])
      testActor = vtk.vtkActor()
      testActor.SetMapper(edgeMapper)
      prop = testActor.GetProperty()
      prop.SetColor(yellow)
      prop.SetLineWidth(3)
      self.renderer.AddActor(testActor)
      self.contourResults[index] = testActor

      # Concatenate and get transform (w,x,y,z)
      userAttempt.GetUserTransform().Concatenate(icp.GetMatrix())

      userAttempt.GetUserTransform().Update()

      (deg, x, y, z) = userAttempt.GetUserTransform().GetOrientationWXYZ()

      resetPositions = self.lastPositions['reset'][index]

      # Not correct!!!
      positionError = np.array(userAttempt.GetUserTransform().GetPosition())

      positionError = np.array(userAttempt.GetUserTransform().TransformPoint(resetPositions[3])) - np.array(resetPositions[3])

      # Reset afterwards
      userAttempt.SetUserTransform(None)
      userAttempt.Modified()

      # Reset planeWidget.
      planeWidget.SetEnabled(0)
      planeWidget.SetOrigin(resetPositions[0])
      planeWidget.SetPoint1(resetPositions[1])
      planeWidget.SetPoint2(resetPositions[2])
      planeWidget.Modified()
      planeWidget.SetEnabled(1)

      lastNormal = planeWidget.GetNormal()
      lastAxis1 = vtk.vtkVector3d()
      vtk.vtkMath.Subtract(planeWidget.GetPoint1(),
                           planeWidget.GetOrigin(),
                           lastAxis1)
      lastOrigin = planeWidget.GetCenter()

      self.lastPositions['origin'][index] = lastOrigin
      self.lastPositions['normal'][index] = lastNormal
      self.lastPositions['axis1'][index] = lastAxis1
      self.render_window.Render()

      originalNormal = resetPositions[4]
      originalNormal = originalNormal / np.sqrt(np.sum(originalNormal**2))
      self.widgetRegistered.emit((deg, np.r_[x,y,z]), originalNormal, positionError)
    elif key == 'm':
      # Move a reference
      print('Moving reference')
      # Remove old reference plane
      self.removeActor(refActor)
      refActor = None

      # Move reference plane
      planeSource.SetOrigin(planeWidget.GetOrigin())
      planeSource.SetPoint1(planeWidget.GetPoint1())
      planeSource.SetPoint2(planeWidget.GetPoint2())

      # mapper
      mapper = vtk.vtkPolyDataMapper()
      mapper.SetInputConnection(planeSource.GetOutputPort())

      # actor
      actor = vtk.vtkActor()
      actor.SetMapper(mapper)
      prop = actor.GetProperty()
      prop.SetColor(blue)
      prop.SetOpacity(self.opacity)

      self.refplanes[index] = actor
      self.renderer.AddActor(actor)

      self.removeActor(contourActor)

      tubes, oldContours = self.computeContoursAndTubes(planeSource)

      edgeMapper = vtk.vtkPolyDataMapper()
      edgeMapper.ScalarVisibilityOff()
      edgeMapper.SetInputConnection(tubes.GetOutputPort())

      planes = self.computeClippingPlanes(planeSource)

      edgeMapper.SetClippingPlanes(planes)

      actor = vtk.vtkActor()
      actor.SetMapper(edgeMapper)
      prop = actor.GetProperty()
      prop.SetColor(green)
      prop.SetLineWidth(3)
      self.contours[index] = actor
      self.renderer.AddActor(actor)

      # User attempt
      self.removeActor(userAttempt)

      attempt = vtk.vtkActor()

      self.fullcontours[index] = oldContours
      mapper = vtk.vtkPolyDataMapper()
      mapper.SetInputData(oldContours)
      attempt.SetMapper(mapper)
      attempt.GetProperty().SetColor(red)
      self.userAttempts[index] = attempt
      self.renderer.AddActor(attempt)

      # Update reset positions of plane widget
      self.lastPositions['reset'][index] = [planeWidget.GetOrigin(),
                                            planeWidget.GetPoint1(),
                                            planeWidget.GetPoint2(),
                                            planeWidget.GetCenter(),
                                            np.array(planeWidget.GetNormal())] # Redundant
      sys.stdout.write("Origin: ")
      print(planeWidget.GetOrigin())
      sys.stdout.write("Point1: ")
      print(planeWidget.GetPoint1())
      sys.stdout.write("Point2: ")
      print(planeWidget.GetPoint2())

      self.render_window.Render()

  def removeActor(self, actor):
    self.interactor.Disable()
    self.renderer.RemoveActor(actor)
    self.interactor.Enable()
    # self.render_window.Render()
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
    """
    Dumped from slicer program
    """
    refplanes = []
    centers = []

    if IOUSFAN:
      refplanes.append(np.array([[1,0,0,-191.357],
                                 [0,0,-1,17.9393],
                                 [0,1,0,192.499],
                                 [0,0,0,1]]))
      centers.append(np.r_[117.54695990115218, 103.95861040356766, 127.81653778974703])
    else:
      refplanes.append(np.array([[0.99722,   -0.00658366270883304, 0.0741938205770847, -321.64],
                                 [0.07419,    0.17584, -0.98162, -227.46],
                                 [-0.0065837, 0.98440,  0.17584, -563.67],
                                 [0,          0,        0,          1.0]]))
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

  def computeClippingPlanes(self, source):
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
    return planes

  def computeContoursAndTubes(self, source):
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
    oldContours = cutStrips.GetOutput()

    tubes = vtk.vtkTubeFilter()
    tubes.SetInputConnection(cutStrips.GetOutputPort())
    tubes.CappingOn()
    tubes.SidesShareVerticesOff()
    tubes.SetNumberOfSides(12)
    tubes.SetRadius(self.worldScale*1.0)
    return tubes, oldContours

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

    # Test position good for slice 17 (HACK)
    if (IOUSFAN):
      source.SetOrigin(-36.00039424299387, 58.447421532729656, 116.93018531955384)
      source.SetPoint1(13.731795848152041, 54.203001711976306, 119.87877296847647)
      source.SetPoint2(-40.18599847580337, 8.635225461941415, 115.82300881527104)
      source.Update()

    self.planeSources.append(source)

    #####################################
    # Blue reference plane
    #####################################

    # mapper
    mapper0 = vtk.vtkPolyDataMapper()
    mapper0.SetInputConnection(source.GetOutputPort())

    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper0)
    prop = actor.GetProperty()
    prop.SetColor(blue)
    prop.SetOpacity(self.opacity)
    self.refplanes.append(actor)
    self.renderer.AddActor(actor)

    #####################################
    # Compute contours, tubes and clipping
    #####################################

    tubes, oldContours = self.computeContoursAndTubes(source)

    edgeMapper = vtk.vtkPolyDataMapper()
    edgeMapper.ScalarVisibilityOff()
    edgeMapper.SetInputConnection(tubes.GetOutputPort())

    planes = self.computeClippingPlanes(source)

    edgeMapper.SetClippingPlanes(planes)

    actor = vtk.vtkActor()
    actor.SetMapper(edgeMapper)
    prop = actor.GetProperty()
    prop.SetColor(green)
    prop.SetLineWidth(3)

    self.contours.append(actor)
    self.renderer.AddActor(actor)

    ###################################################
    # Plane widget for interaction
    ###################################################
    planeWidget = vtk.vtkPlaneWidget()
    planeWidget.SetInteractor(self.interactor)
    planeWidget.SetOrigin(source.GetOrigin())
    planeWidget.SetPoint1(source.GetPoint1())
    planeWidget.SetPoint2(source.GetPoint2())
    prop = planeWidget.GetHandleProperty()
    prop.SetColor(QLiverViewer.colors.GetColor3d("Red"))
    prop = planeWidget.GetPlaneProperty()
    prop.SetColor(QLiverViewer.colors.GetColor3d("Red"))

    # Original position and orientation of reference plane
    self.lastPositions['reset'][index] = [planeWidget.GetOrigin(),
                                          planeWidget.GetPoint1(),
                                          planeWidget.GetPoint2(),
                                          planeWidget.GetCenter(),
                                          np.array(planeWidget.GetNormal())] # Redundant
    print('normal')
    print(planeWidget.GetNormal())
    planeWidget.SetEnabled(1)
    planeWidget.AddObserver(vtk.vtkCommand.EndInteractionEvent, self.onWidgetMoved, 1.0)

    attempt = vtk.vtkActor()

    self.fullcontours.append(oldContours)
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
    self.planeWidgets.append(planeWidget)
    self.render_window.Render()

  def onWidgetMoved(self, obj, ev):
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

      (deg, x, y, z) = self.userAttempts[index].GetUserTransform().GetOrientationWXYZ()

      self.lastPositions['origin'][index] = obj.GetCenter()

      lastAxis1 = [first1[0], first1[1], first1[2]]
      lastNormal = (normal1[0], normal1[1], normal1[2])

      self.lastPositions['axis1'][index] = lastAxis1
      self.lastPositions['normal'][index] = lastNormal

      self.userAttempts[index].Modified()
      self.render_window.Render()

      print("center")

      print('wtf')

      originalNormal = self.lastPositions['reset'][index][4]
      originalNormal = originalNormal / np.sqrt(np.sum(originalNormal**2))

      self.widgetMoved.emit((deg, np.r_[x,y,z]), originalNormal, np.array(obj.GetCenter())-np.array(self.lastPositions['reset'][index][3]))
    #print(ev)

  def initVessels(self):
    qDebug('initVessels()')
    if os.name == 'nt':
      filename = os.path.join(filedir, '../../data/Abdomen/Connected.vtp')
      if IOUSFAN:
        #filename = 'e:/analogic/TrialVTK/data/VesselMeshData.vtk'
        filename = 'e:/analogic/TrialVTK/data/LiverVesselMeshData.vtk'
    else:
      filename = '/home/jmh/bkmedical/data/CT/Connected.vtp'

    # read data
    if IOUSFAN:
      reader = vtk.vtkGenericDataObjectReader()
    else:
      reader = vtk.vtkXMLPolyDataReader()
    reader.SetFileName(filename)
    reader.Update()

    connectFilter = vtk.vtkPolyDataConnectivityFilter()
    connectFilter.SetInputConnection(reader.GetOutputPort())
    connectFilter.SetExtractionModeToLargestRegion()
    connectFilter.Update();

    self.vesselPolyData = self.scale(connectFilter.GetOutput())

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
      filename = os.path.join(filedir, '../../data/Abdomen/Liver_3D-interpolation.vtp')
      if IOUSFAN:
        filename = 'e:/analogic/TrialVTK/data/segmented_liver_ITK_snap.vtk'
    else:
      filename = '/home/jmh/bkmedical/data/CT/Liver_3D-interpolation.vtp'
    if IOUSFAN:
      reader = vtk.vtkGenericDataObjectReader()
    else:
      reader = vtk.vtkXMLPolyDataReader()

    reader.SetFileName(filename)
    reader.Update()

    connectFilter = vtk.vtkPolyDataConnectivityFilter()
    connectFilter.SetInputConnection(reader.GetOutputPort())
    connectFilter.SetExtractionModeToLargestRegion()
    connectFilter.Update();

    surfNormals = vtk.vtkPolyDataNormals()
    surfNormals.SetInputData(self.scale(connectFilter.GetOutput()))

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
    #self.interactor.Start()
