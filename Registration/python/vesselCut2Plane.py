#!/usr/bin/env python

# TODO: Use vtkLinearExtrusionfilter and vtkPolyDataToImagestencil to create mask in 2D

import vtk
import math
from vtk.util.colors import red, blue, black, yellow

filename = '/home/jmh/bkmedical/data/CT/vessels.vtp'

reader = vtk.vtkXMLPolyDataReader()
reader.SetFileName(filename)
reader.Update()

vesselNormals = vtk.vtkPolyDataNormals()
vesselNormals.SetInputConnection(reader.GetOutputPort())

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(vesselNormals.GetOutputPort())
actor = vtk.vtkActor()
actor.SetMapper(mapper)
prop = actor.GetProperty()
prop.SetColor(red)

# Plane for cutting
plane = vtk.vtkPlane()
#plane.SetOrigin(-90,-150, 0)
#plane.SetNormal(1, 0, 0)
plane.SetOrigin(-61.81, -149.31, -203.60)
plane.SetNormal(0, 1, 0)

# -76.95,-146.88, -226.91

# Cutting creates lines where the cut function intersects the model
cutEdges = vtk.vtkCutter()
cutEdges.SetInputConnection(vesselNormals.GetOutputPort())
cutEdges.SetCutFunction(plane)
cutEdges.GenerateCutScalarsOn()
cutEdges.SetValue(0, 0.5)
cutEdges.Update()
print(cutEdges.GetNumberOfContours())

# Stripping to create polylines
cutStrips = vtk.vtkStripper()
cutStrips.SetInputConnection(cutEdges.GetOutputPort())
cutStrips.Update()

circle = cutStrips.GetOutput()

tubes = vtk.vtkTubeFilter()
tubes.SetInputConnection(cutStrips.GetOutputPort()) # works
tubes.CappingOn()
tubes.SidesShareVerticesOff()
tubes.SetNumberOfSides(12)
tubes.SetRadius(1.0)

edgeMapper = vtk.vtkPolyDataMapper()
edgeMapper.ScalarVisibilityOff()
edgeMapper.SetInputConnection(tubes.GetOutputPort())

edgeActor = vtk.vtkActor()
edgeActor.SetMapper(edgeMapper)
edgeActor.GetProperty().SetColor(yellow)
edgeActor.GetProperty().SetLineWidth(3)

cubeAxesActor = vtk.vtkCubeAxesActor()
cubeAxesActor.SetUseTextActor3D(1)
cubeAxesActor.SetBounds(reader.GetOutput().GetBounds())


# Setup a renderer, render window, and interactor
renderer = vtk.vtkRenderer()
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetWindowName("Test")
renderWindow.SetSize(600, 600)

renderWindow.AddRenderer(renderer);
renderWindowInteractor = vtk.vtkRenderWindowInteractor()
renderWindowInteractor.SetRenderWindow(renderWindow)

#Add the actor to the scene
renderer.AddActor(actor)
renderer.AddActor(edgeActor)
renderer.SetBackground(1,1,1) # Background color white
renderer.ResetCamera()

cubeAxesActor.SetCamera(renderer.GetActiveCamera())
cubeAxesActor.XAxisMinorTickVisibilityOff()
cubeAxesActor.YAxisMinorTickVisibilityOff()
#cubeAxesActor.ZAxisMinorTickVisibilityOff()
cubeAxesActor.SetFlyModeToStaticEdges()
for i in range(3):
  cubeAxesActor.GetLabelTextProperty(i).SetColor(black)
  cubeAxesActor.GetTitleTextProperty(i).SetColor(black)
cubeAxesActor.GetXAxesLinesProperty().SetColor(black)
cubeAxesActor.GetYAxesLinesProperty().SetColor(black)
cubeAxesActor.GetZAxesLinesProperty().SetColor(black)

cubeAxesActor.GetProperty().SetColor(black)

renderer.AddActor(cubeAxesActor)

# TEST
if 1:
  whiteImage = vtk.vtkImageData()
  bounds = [0] * 6
  circle.GetBounds(bounds)
  print(bounds)
  spacing = [0] * 3  # desired volume spacing
  spacing[0] = 0.5
  spacing[1] = 0.5
  spacing[2] = 0.5
  whiteImage.SetSpacing(spacing)
  print(spacing)
  # compute dimensions
  dim = [0] * 3
  for i in range(3):
      dim[i] = int(math.ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i])) + 1
      if dim[i] < 1:
          dim[i] = 1
  print(dim)
  whiteImage.SetDimensions(dim)
  whiteImage.SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1)
  origin = [0] * 3
  # NOTE: I am not sure whether or not we had to add some offset!
  origin[0] = bounds[0]  # + spacing[0] / 2
  origin[1] = bounds[2]  # + spacing[1] / 2
  origin[2] = bounds[4]  # + spacing[2] / 2

  whiteImage.SetOrigin(origin)
  whiteImage.AllocateScalars(vtk.VTK_UNSIGNED_CHAR, 1)

  # fill the image with foreground voxels:
  inval = 255
  outval = 0
  count = whiteImage.GetNumberOfPoints()
  # for (vtkIdType i = 0 i < count ++i)
  for i in range(count):
      whiteImage.GetPointData().GetScalars().SetTuple1(i, inval)

  # sweep polygonal data (this is the important thing with contours!)
  extruder = vtk.vtkLinearExtrusionFilter()
  extruder.SetInputData(circle)
  extruder.SetScaleFactor(1.0) # Not working
  # extruder.SetExtrusionTypeToNormalExtrusion()
  extruder.SetExtrusionTypeToVectorExtrusion()
  extruder.SetVector(0, 1, 0)
  extruder.Update()

  # polygonal data -. image stencil:
  pol2stenc = vtk.vtkPolyDataToImageStencil()
  pol2stenc.SetTolerance(0)  # important if extruder.SetVector(0, 0, 1) !!!
  pol2stenc.SetInputConnection(extruder.GetOutputPort())
  pol2stenc.SetOutputOrigin(origin)
  pol2stenc.SetOutputSpacing(spacing)
  pol2stenc.SetOutputWholeExtent(whiteImage.GetExtent())
  pol2stenc.Update()

  # cut the corresponding white image and set the background:
  imgstenc = vtk.vtkImageStencil()
  imgstenc.SetInputData(whiteImage)
  imgstenc.SetStencilConnection(pol2stenc.GetOutputPort())
  imgstenc.ReverseStencilOff() # Was off
  imgstenc.SetBackgroundValue(outval)
  imgstenc.Update()

  imageWriter = vtk.vtkPNGWriter()
  imageWriter.SetFileName("labelImage.png")
  imageWriter.SetInputConnection(imgstenc.GetOutputPort())
  imageWriter.Write()

#Render and interact
renderWindow.Render()

renderWindowInteractor.Start()
