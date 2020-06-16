import os
import vtk
from vtk.util.colors import red, blue, black, yellow

import numpy as np

# TODO: Fast version - make tube filter work
fast = False

def hexCol(s):
  if isinstance(s,str):
    if "#" in s:  # hex to rgb
      h = s.lstrip("#")
      rgb255 = list(int(h[i : i + 2], 16) for i in (0, 2, 4))
      rgbh = np.array(rgb255) / 255.0
      return tuple(rgbh)

colors = vtk.vtkNamedColors()


# create a rendering window and renderer
ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.SetWindowName("Reference Planes")
renWin.SetSize(800,800)
renWin.AddRenderer(ren)

# create a renderwindowinteractor
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# Load reference data set to get dimensionality
#volumeReader = vtk.vtkMetaImageReader()
#volumeReader.SetFileName('/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd')
#volumeReader.Update()
#dims = volumeReader.GetOutput().GetDimensions()

# read vessel data
if os.name == 'nt':
  filename = 'e:/analogic/TrialVTK/data/Abdomen/A.vtp'
  fileLiver = 'e:/analogic/TrialVTK/data/Abdomen/Liver_3D-interpolation.vtp'
else:
  filename = '/home/jmh/bkmedical/data/CT/Connected.vtp'
  fileLiver = '/home/jmh/bkmedical/data/CT/Liver_3D-interpolation.vtp'

# read liver data
readerLiver = vtk.vtkXMLPolyDataReader()
readerLiver.SetFileName(fileLiver)
readerLiver.Update()

surfNormals = vtk.vtkPolyDataNormals()
surfNormals.SetInputConnection(readerLiver.GetOutputPort())


#Create a mapper and actor
mapperLiver = vtk.vtkPolyDataMapper()
mapperLiver.SetInputConnection(surfNormals.GetOutputPort()) # was readerLiver
actorLiver = vtk.vtkActor()
actorLiver.SetMapper(mapperLiver)
prop = actorLiver.GetProperty()
#prop.SetColor(colors.GetColor3d("Purple"))

#prop.SetColor(vtk.vtkColor3d(hexCol("#6c2e1f")))
prop.SetColor(vtk.vtkColor3d(hexCol("#873927")))  # (25% lighter)
prop.SetOpacity(0.5)


# read data
reader = vtk.vtkXMLPolyDataReader()
reader.SetFileName(filename)
reader.Update()

# compute normals
vesselNormals = vtk.vtkPolyDataNormals()
vesselNormals.SetInputConnection(reader.GetOutputPort())

# mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(vesselNormals.GetOutputPort())

# actor for vessels
actor = vtk.vtkActor()
actor.SetMapper(mapper)
prop = actor.GetProperty()
#prop.SetColor(red)
#prop.SetColor(vtk.vtkColor3d(hexCol("#415d6c")))
prop.SetColor(vtk.vtkColor3d(hexCol("#517487"))) # 25% lighter

# assign actor to the renderer
ren.AddActor(actor)
ren.AddActor(actorLiver)

# loop over reference planes
refplanes = []


refplanes.append(np.array([[0.9972221078443415,	-0.00658366270883304,	0.0741938205770847,	-321.63981996062756],
                           [0.07419382057708453,	0.1758409118814136,	-0.9816186890523674,	-227.4617768729491],
                           [-0.006583662708833091,	0.9843965812080258,	0.1758409118814139,	-563.6745122183239],
                           [0,	0,	0,	1]]))

centers = []
centers.append(np.r_[-75.69560243429154,	-149.41579842495196,	-231.76170945617494])

refplanes.append(np.array([[0.9824507428729312,	-0.028608856565971154,	0.1843151408713164,	-221.425151769367],
                           [0.18431514087131629,	0.3004711475787132,	-0.935812491003576,	-325.6553959586223],
                           [-0.028608856565971223,	0.9533617481306448,	0.3004711475787133,	-547.1574253306663],
                           [0,	0,	0,	1]]))

centers.append(np.r_[-31.317285034663634,	-174.62449255285645,	-193.39018826551072])

hw = 50.0
#mappers = []
actors = []
edgeActors = []
lineActors = []
intActors = []

nPlanes = 1
nPlanes = len(refplanes)
for iPlane in range(nPlanes):
  source = vtk.vtkPlaneSource()
  source.SetOrigin(0,  0, 0)
  source.SetPoint1(hw, 0, 0)
  source.SetPoint2(0, hw, 0)

  transform = vtk.vtkTransform()
  mat = vtk.vtkMatrix4x4()
  for i in range(4):
    for j in range(4):
      mat.SetElement(i, j, refplanes[iPlane][i,j])

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

  # TEST
  source.SetCenter(centers[iPlane])
  source.Update()

  # mapper
  mapper0 = vtk.vtkPolyDataMapper()
  mapper0.SetInputConnection(source.GetOutputPort())

  # actor
  actors.append(vtk.vtkActor())
  actors[iPlane].SetMapper(mapper0)
  prop = actors[iPlane].GetProperty()
  prop.SetColor(blue)
  prop.SetOpacity(0.5)

  plane = vtk.vtkPlane()
  plane.SetOrigin(source.GetOrigin())
  plane.SetNormal(source.GetNormal())

  if not(fast):
    # vtkPlaneCutter is faster
    cutEdges = vtk.vtkCutter()
    cutEdges.SetInputConnection(vesselNormals.GetOutputPort())
    cutEdges.SetCutFunction(plane)
    cutEdges.GenerateCutScalarsOn()
    cutEdges.SetValue(0, 0.5)
    cutEdges.Update()
  else:
    # TODO Add tube filter
    cutEdges = vtk.vtkPlaneCutter()
    cutEdges.SetPlane(plane)
    cutEdges.SetComputeNormals(1)
    cutEdges.SetInputConnection(reader.GetOutputPort())
    cutEdges.Update()

    sCutterMapper = vtk.vtkCompositePolyDataMapper()
    sCutterMapper.SetInputConnection(cutEdges.GetOutputPort())
    sCutterMapper.ScalarVisibilityOff()

    sCutterActor = vtk.vtkActor()
    sCutterActor.SetMapper(sCutterMapper)

    # vtkPointLocator and get surface contour

  cutStrips = vtk.vtkStripper()
  cutStrips.SetInputConnection(cutEdges.GetOutputPort())
  cutStrips.Update()

  tubes = vtk.vtkTubeFilter()
  tubes.SetInputConnection(cutStrips.GetOutputPort()) # works
  tubes.CappingOn()
  tubes.SidesShareVerticesOff()
  tubes.SetNumberOfSides(12)
  tubes.SetRadius(1.0)

  edgeMapper = vtk.vtkPolyDataMapper()
  edgeMapper.ScalarVisibilityOff()
  edgeMapper.SetInputConnection(tubes.GetOutputPort())

  # Find nearest point on surface
  cutSurf = vtk.vtkCutter()
  cutSurf.SetInputConnection(surfNormals.GetOutputPort())
  cutSurf.SetCutFunction(plane)
  cutSurf.GenerateCutScalarsOn()
  cutSurf.SetValue(0, 0.5)
  cutSurf.Update()

  # Find nearest point
  cutStrips0 = vtk.vtkStripper()
  cutStrips0.SetInputConnection(cutSurf.GetOutputPort())
  cutStrips0.Update()

  # Is empty
  surfCircle = cutSurf.GetOutput() # vtkPolyData


  # Visualizer surface intersection
  tubes0 = vtk.vtkTubeFilter()
  tubes0.SetInputConnection(cutStrips0.GetOutputPort()) # works
  tubes0.CappingOn()
  tubes0.SidesShareVerticesOff()
  tubes0.SetNumberOfSides(12)
  tubes0.SetRadius(1.0)

  edgeMapper0 = vtk.vtkPolyDataMapper()
  edgeMapper0.ScalarVisibilityOff()
  edgeMapper0.SetInputConnection(tubes0.GetOutputPort())

  intActors.append(vtk.vtkActor())
  intActors[iPlane].SetMapper(edgeMapper0)


  if 1:
    points = surfCircle.GetPoints()
    pointTree = vtk.vtkKdTree()
    pointTree.BuildLocatorFromPoints(points)

    nClosest = 3
    result = vtk.vtkIdList()
    pointTree.FindClosestNPoints(nClosest+1, centers[iPlane], result)
    #dataArray = points.GetData()
    #x = dataArray.GetComponent(result.GetId(2), 0)
    #y = dataArray.GetComponent(result.GetId(0), 1)
    #z = dataArray.GetComponent(result.GetId(0), 2)

    #print(x,y,z)
    print(points.GetPoint(result.GetId(nClosest)))
    # Visualize line
    #lineSource = vtk.vtkLineSource()
    lineSource = vtk.vtkSphereSource()
    #lineSource.SetPoint1((x,y,z))
    #lineSource.SetPoint2(source.GetCenter())
    lineSource.SetCenter(points.GetPoint(result.GetId(nClosest)))
    lineSource.SetRadius(5)
    lineSource.Update()

    lineMapper = vtk.vtkPolyDataMapper()
    lineMapper.SetInputConnection(lineSource.GetOutputPort())
    lineActors.append(vtk.vtkActor())
    lineActors[iPlane].SetMapper(lineMapper)
    prop = lineActors[iPlane].GetProperty()
    prop.SetLineWidth(20)
    prop.SetColor(colors.GetColor3d("Red"))

    # Dist plane center to intersection
    #d2p = np.sqrt(np.sum((np.array(source.GetCenter()) - np.r_[x,y,z])**2))
    #print(d2p)

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

  edgeActors.append(vtk.vtkActor())
  edgeActors[iPlane].SetMapper(edgeMapper)
  edgeActors[iPlane].GetProperty().SetColor(yellow)
  edgeActors[iPlane].GetProperty().SetLineWidth(3)

  if fast:
    ren.AddActor(sCutterActor)
  else:
    ren.AddActor(actors[iPlane])
    ren.AddActor(edgeActors[iPlane])

  ren.AddActor(intActors[iPlane])
  ren.AddActor(lineActors[iPlane])

ren.SetBackground(1,1,1)
ren.ResetCamera()


# enable user interface interactor
iren.Initialize()
renWin.Render()
iren.Start()

# Local variables: #
# tab-width: 2 #
# python-indent: 2 #
# python-indent-offset: 2 #
# indent-tabs-mode: nil #
# End: #
