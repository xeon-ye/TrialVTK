import vtk

renderer = vtk.vtkRenderer()
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetWindowName("Test")
renderWindow.SetSize(600, 600)

renderWindow.AddRenderer(renderer);
renderWindowInteractor = vtk.vtkRenderWindowInteractor()
renderWindowInteractor.SetRenderWindow(renderWindow)

planeWidget = vtk.vtkPlaneWidget()
planeWidget.SetInteractor(renderWindowInteractor)

planeWidget.On()

renderWindowInteractor.Initialize()

renderer.ResetCamera()
renderWindow.Render()
renderWindowInteractor.Start()
