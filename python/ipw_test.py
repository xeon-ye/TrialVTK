import tempfile
import vtk

from scipy import misc
from PIL import Image

fileName = tempfile.mkstemp( suffix = '.png' )[1]

Image.fromarray(misc.face()).save( fileName )

reader = vtk.vtkPNGReader()
reader.SetFileName( fileName )
reader.Update()

actor = vtk.vtkImageActor()
actor.SetInputData( reader.GetOutput() )

ren = vtk.vtkRenderer()
ren.AddActor( actor )

renWin = vtk.vtkRenderWindow()
renWin.AddRenderer( ren )

iren = vtk.vtkRenderWindowInteractor()
iren.SetRenderWindow( renWin )
iren.Initialize()

iStyle = vtk.vtkInteractorStyleImage()
iren.SetInteractorStyle( iStyle )

if 0:
  planeWidgetX = vtk.vtkImagePlaneWidget()
  planeWidgetX.SetInteractor( iren )
  planeWidgetX.RestrictPlaneToVolumeOn()
  planeWidgetX.SetResliceInterpolateToNearestNeighbour()
  planeWidgetX.SetInputData( reader.GetOutput() )
  planeWidgetX.SetPlaneOrientationToZAxes()
  planeWidgetX.SetSliceIndex( 0 )
  planeWidgetX.DisplayTextOn()
  planeWidgetX.SetResliceInterpolateToNearestNeighbour()
  planeWidgetX.On()


ren.AddActor( actor )
renWin.SetSize( 400, 400 )
renWin.Render()
ren.ResetCameraClippingRange()
renWin.Render()

iren.Start()
