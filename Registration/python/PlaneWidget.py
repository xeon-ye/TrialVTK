import vtk
import numpy as np
import sys

global lastNormal
global lastAxis1
global lastAxis2
global axes

def MyFunc(obj, ev):
  global lastNormal
  global lastAxis1
  global axes

  saxis = np.array(obj.GetPoint1()) - np.array(obj.GetOrigin())
  taxis = np.array(obj.GetPoint2()) - np.array(obj.GetOrigin())
  axis = np.array(obj.GetNormal())

  if 0:
    sys.stdout.write('lastNormal: ')
    print(lastNormal)
    sys.stdout.write('lastAxis1: ')
    print(lastAxis1)
    sys.stdout.write('newNormal: ')
    print(axis)
    sys.stdout.write('newAxis1: ')
    print(saxis)

  # TODO: Ensure axis,taxis and saxis are e.g. [0,1,0], [0,0,1] and
  # [1,0,0] if normal is [0.48, 0.84, -0.21]

  # Below constrain rotation of 4x4 such that the axis of rotation
  # is parallel to the plane (so there is no in-plane rotation)

  vec = vtk.vtkVector3d() # Axis of rotation
  vtk.vtkMath.Cross(lastNormal, axis, vec)
  costheta = vtk.vtkMath.Dot(axis, lastNormal)
  sintheta = vtk.vtkMath.Norm(vec)
  theta = np.arctan2(sintheta, costheta)

  if sintheta != 0.0:
    vec[0] = vec[0]/sintheta
    vec[1] = vec[1]/sintheta
    vec[2] = vec[2]/sintheta

  # Convert to Quaternion
  costheta = np.cos(0.5*theta)
  sintheta = np.sin(0.5*theta)
  quat = vtk.vtkQuaterniond(costheta, vec[0]*sintheta, vec[1]*sintheta, vec[2]*sintheta)
  rot0 = np.ones((3,3),dtype=np.float)
  vtk.vtkMath.QuaternionToMatrix3x3(quat, rot0)

  #vtkMath.vtkMultiply3x3(mat, lastAxis2, newAxis2)

  if 1:
    newAxis1 = vtk.vtkVector3d()
    vtk.vtkMath.Multiply3x3(rot0, lastAxis1, newAxis1)

    # Rotate newAxis1 into saxis
    vec = vtk.vtkVector3d() # Axis of rotation
    vtk.vtkMath.Cross(newAxis1, saxis, vec)
    costheta = vtk.vtkMath.Dot(saxis, newAxis1)
    sintheta = vtk.vtkMath.Norm(vec)
    theta = np.arctan2(sintheta, costheta)
    if sintheta != 0.0:
      vec[0] = vec[0]/sintheta
      vec[1] = vec[1]/sintheta
      vec[2] = vec[2]/sintheta

    # Convert to Quaternion
    costheta = np.cos(0.5*theta)
    sintheta = np.sin(0.5*theta)
    quat = vtk.vtkQuaterniond(costheta, vec[0]*sintheta, vec[1]*sintheta, vec[2]*sintheta)
    rot1 = np.ones((3,3),dtype=np.float)
    vtk.vtkMath.QuaternionToMatrix3x3(quat, rot1)

    # Concatenate rotations
    rot = np.dot(rot1, rot0)

  mat = np.zeros((4,4), dtype=np.float)
  mat[:3,:3] = rot
  mat[3,3] = 1.0


  # TEST (why is this working)
  tmp = vtk.vtkVector3d()
  vtk.vtkMath.Multiply3x3(rot, axes.GetOrigin(), tmp)
  mat[:3,3] = np.array(obj.GetCenter()) - np.array(tmp)


  # Construct 4x4 matrix
  trans = vtk.vtkMatrix4x4()
  trans.DeepCopy(mat.flatten().tolist())

  if axes.GetUserTransform() is not None:
    axes.GetUserTransform().Concatenate(trans)
  else:
    transform = vtk.vtkTransform()
    transform.SetMatrix(trans)
    transform.PostMultiply()
    axes.SetUserTransform(transform)

  # Only for book keeping
  axes.SetOrigin(obj.GetCenter()) # Not modified by SetUserTransform
  axes.Modified()

  lastAxis1[0] = saxis[0]
  lastAxis1[1] = saxis[1]
  lastAxis1[2] = saxis[2]

  lastNormal = (axis[0], axis[1], axis[2])


renderer = vtk.vtkRenderer()
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetWindowName("Test")
renderWindow.SetSize(600, 600)

renderWindow.AddRenderer(renderer);
renderWindowInteractor = vtk.vtkRenderWindowInteractor()
renderWindowInteractor.SetRenderWindow(renderWindow)

planeWidget = vtk.vtkPlaneWidget()
planeWidget.SetInteractor(renderWindowInteractor)
planeWidget.AddObserver(vtk.vtkCommand.EndInteractionEvent, MyFunc, 1.0)

# FIX THIS

initialMovement = True

if initialMovement:
  planeWidget.SetOrigin(1.5, 0.5, 2)
  planeWidget.SetPoint1(2.5, 0.5, 2)
  planeWidget.SetPoint2(1.5, 1.5, 2)

planeWidget.Modified()
planeWidget.On()

lastNormal = planeWidget.GetNormal()
lastAxis1 = vtk.vtkVector3d()
lastAxis2 = vtk.vtkVector3d()

vtk.vtkMath.Subtract(planeWidget.GetPoint1(),
                     planeWidget.GetOrigin(),
                     lastAxis1)
vtk.vtkMath.Subtract(planeWidget.GetPoint2(),
                     planeWidget.GetOrigin(),
                     lastAxis2)


renderWindowInteractor.Initialize()

renderer.ResetCamera()

# Crazy behavior for origin
axes = vtk.vtkAxesActor()
if initialMovement:
  tf = vtk.vtkTransform()

  rot = np.diag(np.ones(3, dtype=np.float))
  mat = np.zeros((4,4), dtype=np.float)
  mat[:3,:3] = rot
  mat[3,3] = 1.0
  tmp = vtk.vtkVector3d()
  vtk.vtkMath.Multiply3x3(rot, axes.GetOrigin(), tmp)
  mat[:3,3] = np.array(planeWidget.GetCenter()) - np.array(tmp)

  # Construct 4x4 matrix
  tfm = vtk.vtkMatrix4x4()
  tfm.DeepCopy(mat.flatten().tolist())
  tfm.Modified()
  tf.SetMatrix(tfm)
  tf.PostMultiply()
  tf.Update()
  axes.SetUserTransform(tf)
  axes.SetOrigin(planeWidget.GetCenter())
  axes.Modified()

renderer.AddActor(axes)

renderWindow.Render()
renderWindowInteractor.Start()
