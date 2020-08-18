from __future__ import print_function
import vtk

def get_program_parameters():
  import argparse
  description = 'Read a polydata file.'
  epilogue = ''''''
  parser = argparse.ArgumentParser(description=description, epilog=epilogue,
                                   formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument('filename', help='vessels.vtp')

  parser.add_argument('--decimation', type=float, default=None, help='(default: 1.0 - no decimation)')
  parser.add_argument('--wireframe', type=int, default=0)
  args = parser.parse_args()
  return args.filename, args.wireframe, args.decimation

def main():
   colors = vtk.vtkNamedColors()

   filename, wireframe, factor = get_program_parameters()

   # Read all the data from the file
   reader = vtk.vtkXMLPolyDataReader()
   reader.SetFileName(filename)
   reader.Update()

   inputPolyData = reader.GetOutput()

   #Create a mapper and actor
   mapper = vtk.vtkPolyDataMapper()

   if factor is not None:
     triangles = vtk.vtkTriangleFilter()
     triangles.SetInputConnection(reader.GetOutputPort());
     triangles.Update();
     inputPolyData = triangles.GetOutput();
     reduction = factor
     decPro = vtk.vtkDecimatePro()
     decPro.SetInputData(inputPolyData)
     decPro.SetTargetReduction(reduction)
     decPro.PreserveTopologyOn()
     decPro.Update()
     mapper.SetInputData(decPro.GetOutput())
   else:
     mapper.SetInputConnection(reader.GetOutputPort())
   actor = vtk.vtkActor()
   actor.SetMapper(mapper)

   if (wireframe):
    actor.GetProperty().SetLineWidth(3)
    actor.GetProperty().SetRepresentationToWireframe()

    actor.GetProperty().SetEdgeVisibility(1)
    actor.GetProperty().SetEdgeColor(0.9,0.9,0.4)
    actor.GetProperty().SetLineWidth(6)
    actor.GetProperty().SetPointSize(12)
    actor.GetProperty().SetRenderLinesAsTubes(1)
    actor.GetProperty().SetRenderPointsAsSpheres(1)
    actor.GetProperty().SetVertexVisibility(1)
    actor.GetProperty().SetVertexColor(0.5,1.0,0.8)

   prop = actor.GetProperty()
   prop.SetColor(colors.GetColor3d("Red"))

   # Setup a renderer, render window, and interactor
   renderer = vtk.vtkRenderer()
   renderWindow = vtk.vtkRenderWindow()
   renderWindow.SetWindowName("Test")
   renderWindow.SetSize(600, 600)

   renderWindow.AddRenderer(renderer)
   renderWindowInteractor = vtk.vtkRenderWindowInteractor()
   renderWindowInteractor.SetRenderWindow(renderWindow)

   #Add the actor to the scene
   renderer.AddActor(actor)
   renderer.SetBackground(1,1,1) # Background color white
   renderer.ResetCamera()

   #Render and interact
   renderWindow.Render()

   renderWindowInteractor.Start()


if __name__ == '__main__':
   main()
