import vtk
import numpy as np
import open3d as o3d

from timeit import default_timer as timer

# Experiment importing o3d data and applying VTK's IterativeClosestPoint

# Using VTK in a final application, the points already reside on the GPU

source = o3d.io.read_point_cloud("./vesseltree.pcd")
target = o3d.io.read_point_cloud("./vesselplane.pcd")

colors = vtk.vtkNamedColors()


# ============ create source points ==============
print("Creating source points...")
sourcePoints = vtk.vtkPoints()
sourceVertices = vtk.vtkCellArray()

points = np.asarray(source.points)

# InsertPoints(vtkIdList dstId, vtkIdList srcId, vtkPoints)
# InsertPoints(int dstStartId, int vtkIdType, int srStartId, vtkPoints)

for i in range(len(points)):
  id = sourcePoints.InsertNextPoint(points[i,:])
  sourceVertices.InsertNextCell(1) # 1 is vtkIdType
  sourceVertices.InsertCellPoint(id)

src = vtk.vtkPolyData()
src.SetPoints(sourcePoints)
src.SetVerts(sourceVertices)

# ============ create target points ==============
print("Creating target points...")
targetPoints = vtk.vtkPoints()
targetVertices = vtk.vtkCellArray()
points = np.asarray(target.points)

# InsertPoints(vtkIdList dstId, vtkIdList srcId, vtkPoints)
# InsertPoints(int dstStartId, int vtkIdType, int srStartId, vtkPoints)

# Perform bad alignment

zrot = 3.0/180.0 * np.pi
yoff = 20.0
yoff = 0.0
trans = np.asarray([[np.cos(zrot),  np.sin(zrot), 0.0, 0.0],
                    [-np.sin(zrot),  np.cos(zrot), 0.0, yoff],
                    [0.0, 0.0, 1.0, 0.0],
                    [0.0, 0.0, 0.0, 1.0]])

transform = vtk.vtkTransform()
mat = vtk.vtkMatrix4x4()
for i in range(4):
  for j in range(4):
    mat.SetElement(i, j, trans[i,j])

transform.SetMatrix(mat)
transform.Update()

for i in range(len(points)):
  id = targetPoints.InsertNextPoint(transform.TransformPoint(points[i,:]))
  targetVertices.InsertNextCell(1) # 1 is vtkIdType
  targetVertices.InsertCellPoint(id)

dst = vtk.vtkPolyData()
dst.SetPoints(targetPoints)
dst.SetVerts(targetVertices)



# ============ run ICP ==============
icp = vtk.vtkIterativeClosestPointTransform()
icp.SetSource(dst)
icp.SetTarget(src)
icp.GetLandmarkTransform().SetModeToRigidBody()
icp.DebugOn()
icp.SetMaximumNumberOfIterations(10)
icp.StartByMatchingCentroidsOff()
#icp.SetMeanDistanceModeToRMS()
icp.SetMeanDistanceModeToAbsoluteValue()

#icp.StartByMatchingCentroidsOn()
icp.Modified()
start = timer()
icp.Update()
elapsed = timer() - start
print('elapsed: %f' % (elapsed))

icpTransformFilter = vtk.vtkTransformPolyDataFilter()
icpTransformFilter.SetInputData(dst)
icpTransformFilter.SetTransform(icp)
icpTransformFilter.Update()

tf = icpTransformFilter.GetOutput()

renderer = vtk.vtkRenderer()
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetWindowName("Test")
renderWindow.SetSize(1024,1024)

renderWindow.AddRenderer(renderer);
renderWindowInteractor = vtk.vtkRenderWindowInteractor()
renderWindowInteractor.SetRenderWindow(renderWindow)

# Display src, dst and tf
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputData(src)
actorSrc = vtk.vtkActor()
actorSrc.SetMapper(mapper)
prop = actorSrc.GetProperty()
prop.SetColor(colors.GetColor3d("Red"))

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputData(dst)
actorDst = vtk.vtkActor()
actorDst.SetMapper(mapper)
prop = actorDst.GetProperty()
prop.SetColor(colors.GetColor3d("Blue"))

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputData(tf)
actorTf = vtk.vtkActor()
actorTf.SetMapper(mapper)
prop = actorTf.GetProperty()
prop.SetColor(colors.GetColor3d("Green"))

renderer.AddActor(actorSrc)
renderer.AddActor(actorDst)
renderer.AddActor(actorTf)

renderer.SetBackground(1,1,1) # Background color white
renderer.ResetCamera()

#Render and interact
renderWindow.Render()
renderWindowInteractor.Start()
