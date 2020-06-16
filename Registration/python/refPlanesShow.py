import os
import vtk
from vtk.util.colors import red, blue, black, yellow

import numpy as np

# TODO: Fast version - make tube filter work
fast = False


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
else:
  filename = '/home/jmh/bkmedical/data/CT/Connected.vtp'

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
prop.SetColor(red)


# assign actor to the renderer
ren.AddActor(actor)

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

for iPlane in range(len(refplanes)):
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
  #cutStrips = cutEdges

  tubes = vtk.vtkTubeFilter()
  tubes.SetInputConnection(cutStrips.GetOutputPort()) # works
  tubes.CappingOn()
  tubes.SidesShareVerticesOff()
  tubes.SetNumberOfSides(12)
  tubes.SetRadius(1.0)

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

  edgeActors.append(vtk.vtkActor())
  edgeActors[iPlane].SetMapper(edgeMapper)
  edgeActors[iPlane].GetProperty().SetColor(yellow)
  edgeActors[iPlane].GetProperty().SetLineWidth(3)

  if fast:
    ren.AddActor(sCutterActor)
  else:
    ren.AddActor(actors[iPlane])
    ren.AddActor(edgeActors[iPlane])

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