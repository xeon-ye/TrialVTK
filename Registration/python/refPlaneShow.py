import os
import vtk
import math
from vtk.util.colors import red, blue, black, yellow
import numpy as np

iPlane = 0

# RAS = LPS (first two axes flipped)

# Red plane in slicer
redplane = np.array([[0,0,1,4.99987],
                     [1,0,0,-302.671],
                     [0,1,0,-570.6],
                     [0,0,0,1]])

# x is unchanged y->-z, z->y (RAS->LPS)
greenplane = np.array([[1,0,0,-163.171],
                       [0,0,-1,-134.5],
                       [0,1,0,-570.6],
                       [0,0,0,1]])

blueplane = np.array([[1,0,0,-163.171],
                      [0,1,0,-302.671],
                      [0,0,1,-358.1],
                      [0,0,0,1]])


bifurcation = np.array([[0, 0.504867,  0.863197,  -246.731],
                        [1, 0,         0,         -337.073],
                        [0, 0.863197, -0.5004867, -520.683],
                        [0,0,0,1]])

if iPlane == 0:
  redplane = redplane
elif iPlane == 1:
  redplane = greenplane
else:
  redplane = blueplane

if 0:
  # RAS to LPS
  redplane = np.dot(redplane, np.array([[-1,0,0,0],
                                        [0,-1,0,0],
                                        [0,0,1,0],
                                        [0,0,0,1]]))




# create a rendering window and renderer
ren = vtk.vtkRenderer()
renWin = vtk.vtkRenderWindow()
renWin.SetWindowName("Reference Plane")
renWin.SetSize(800,800)
renWin.AddRenderer(ren)

# create a renderwindowinteractor
iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# read data
if os.name == 'nt':
  filename = 'e:/analogic/TrialVTK/data/Abdomen/A.vtp'
else:
  filename = '/home/jmh/bkmedical/data/CT/Connected.vtp'

# read data
reader = vtk.vtkXMLPolyDataReader()
reader.SetFileName(filename)
reader.Update()

vesselNormals = vtk.vtkPolyDataNormals()
vesselNormals.SetInputConnection(reader.GetOutputPort())

mapper0 = vtk.vtkPolyDataMapper()
mapper0.SetInputConnection(vesselNormals.GetOutputPort())
actor0 = vtk.vtkActor()
actor0.SetMapper(mapper0)
prop0 = actor0.GetProperty()
prop0.SetColor(red)


# create source
source1 = vtk.vtkPlaneSource()

if iPlane == 0:
  ii = np.r_[0,0,1]
  jj = np.r_[1,0,0]
  kk = np.cross(ii,jj)
elif iPlane == 1:
  ii = np.r_[1,0,0]
  jj = np.r_[0,1,0]
  kk = np.cross(ii,jj)
else:
  ii = np.r_[1,0,0]
  jj = np.r_[0,1,0]
  kk = np.cross(ii,jj)

if 0:
  # Works for iPlane==2
  center = redplane[:3,3]
  source1.SetOrigin(center.tolist())
  p1 = center + 500*ii
  p2 = center + 500*jj
  source1.SetPoint1(p1.tolist())
  source1.SetPoint2(p2.tolist())
  print(source1.GetCenter())
else:
  # Works for iPlane==2
  source1.SetOrigin(0,0,0)
  source1.SetPoint1(500,0,0)
  source1.SetPoint2(0,500,0)

  transform = vtk.vtkTransform()
  mat = vtk.vtkMatrix4x4()
  for i in range(4):
    for j in range(4):
      mat.SetElement(i, j, redplane[i,j])

  transform.SetMatrix(mat)
  transform.Update()

  origin = source1.GetOrigin()
  origin = transform.TransformPoint(origin)
  source1.SetOrigin(origin)

  p1 = source1.GetPoint1()
  p1 = transform.TransformPoint(p1)
  source1.SetPoint1(p1)

  p2 = source1.GetPoint2()
  p2 = transform.TransformPoint(p2)
  source1.SetPoint2(p2)
  source1.Update()

# transform plane

if 0:
  center = np.r_[0,0,0]
  p1 = center + 500*ii
  p2 = center + 500*jj

  source1.SetPoint1(p1.tolist())
  source1.SetPoint2(p2.tolist())

  transform = vtk.vtkTransform()
  mat = vtk.vtkMatrix4x4()
  for i in range(4):
    for j in range(4):
      mat.SetElement(j,i, redplane[i,j])

  transform.SetMatrix(mat)
  #transform.Inverse()

  center = source1.GetCenter()
  center = transform.TransformPoint(center)
  source1.SetCenter(center)

  p1 = source1.GetPoint1()
  p1 = transform.TransformPoint(p1)
  source1.SetPoint1(p1)

  p2 = source1.GetPoint2()
  p2 = transform.TransformPoint(p2)
  source1.SetPoint2(p2)

# mapper
mapper1 = vtk.vtkPolyDataMapper()
mapper1.SetInputConnection(source1.GetOutputPort())

# actor
actor1 = vtk.vtkActor()
actor1.SetMapper(mapper1)

# assign actor to the renderer
ren.AddActor(actor0)
ren.AddActor(actor1)
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
