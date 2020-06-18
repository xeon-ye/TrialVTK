import vtk
import numpy as np

def MyFunc(obj, ev):
  # Out-of-plane rotation
  saxis = np.array(obj.GetPoint1()) - np.array(obj.GetOrigin())
  taxis = np.array(obj.GetPoint2()) - np.array(obj.GetOrigin())
  axis = np.array(obj.GetNormal())

  # TODO: Ensure axis,taxis and saxis are e.g. [0,1,0], [0,0,1] and
  # [1,0,0] if normal is [0.48, 0.84, -0.21]

  # Below constrain rotation of 4x4 such that the axis of rotation
  # is parallel to the plane (so there is no in-plane rotation)

  normal = (0,0,1) # Old normal
  vec = vtk.vtkVector3d() # Axis of rotation
  vtk.vtkMath.Cross(axis,normal,vec)
  costheta = vtk.vtkMath.Dot(axis, normal)
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
  mat = np.ones((3,3),dtype=np.float)
  vtk.vtkMath.QuaternionToMatrix3x3(quat, mat)

  # Construct 4x4 matrix
  v1 = vtk.vtkVector3d()
  v2 = vtk.vtkVector3d()
  vtk.vtkMath.Multiply3x3(mat, saxis, v1)
  vtk.vtkMath.Multiply3x3(mat, taxis, v2)

  trans = vtk.vtkMatrix4x4()
  trans.SetElement(0,0,v1[0])
  trans.SetElement(1,0,v1[1])
  trans.SetElement(2,0,v1[2])
  trans.SetElement(3,0,0.0)

  trans.SetElement(0,1,v2[0])
  trans.SetElement(1,1,v2[1])
  trans.SetElement(2,1,v2[2])
  trans.SetElement(3,1,0.0)

  trans.SetElement(0,2,normal[0])
  trans.SetElement(1,2,normal[1])
  trans.SetElement(2,2,normal[2])
  trans.SetElement(3,2,0.0)

  # 4x4 with translation = input center - Rot x output center

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

planeWidget.On()

renderWindowInteractor.Initialize()

renderer.ResetCamera()
renderWindow.Render()
renderWindowInteractor.Start()
