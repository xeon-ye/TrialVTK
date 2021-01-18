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
#cubeAxesActor.SetFlyModeToOuterEdges()
#cubeAxesActor.SetFlyModeToStaticEdges()
cubeAxesActor.SetFlyMode(cubeAxesActor.VTK_FLY_FURTHEST_TRIAD)
for i in range(3):
  cubeAxesActor.GetLabelTextProperty(i).SetColor(black)
  cubeAxesActor.GetTitleTextProperty(i).SetColor(black)
cubeAxesActor.GetXAxesLinesProperty().SetColor(black)
cubeAxesActor.GetYAxesLinesProperty().SetColor(black)
cubeAxesActor.GetZAxesLinesProperty().SetColor(black)

cubeAxesActor.GetProperty().SetColor(black)
cubeAxesActor.DrawXGridlinesOn()
cubeAxesActor.DrawYGridlinesOn()
cubeAxesActor.DrawZGridlinesOn()
cubeAxesActor.GetXAxesGridlinesProperty().SetColor(black)
cubeAxesActor.GetYAxesGridlinesProperty().SetColor(black)
cubeAxesActor.GetZAxesGridlinesProperty().SetColor(black)
cubeAxesActor.SetGridLineLocation(cubeAxesActor.VTK_GRID_LINES_FURTHEST)
cubeAxesActor.Modified()

renderer.AddViewProp(cubeAxesActor)

#Render and interact
renderWindow.Render()

renderWindowInteractor.Start()
