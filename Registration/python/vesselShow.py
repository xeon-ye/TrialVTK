from __future__ import print_function
import vtk

def get_program_parameters():
  import argparse
  description = 'Read a polydata file.'
  epilogue = ''''''
  parser = argparse.ArgumentParser(description=description, epilog=epilogue,
                                   formatter_class=argparse.RawDescriptionHelpFormatter)
  parser.add_argument('filename', help='vessels.vtp')
  args = parser.parse_args()
  return args.filename

def main():
   colors = vtk.vtkNamedColors()

   filename = get_program_parameters()

   print(filename)

   # Read all the data from the file
   reader = vtk.vtkXMLPolyDataReader()
   reader.SetFileName(filename)
   reader.Update()


   #Create a mapper and actor
   mapper = vtk.vtkPolyDataMapper()
   mapper.SetInputConnection(reader.GetOutputPort())
   actor = vtk.vtkActor()
   actor.SetMapper(mapper)
   prop = actor.GetProperty()
   prop.SetColor(colors.GetColor3d("Red"))

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
   renderer.SetBackground(1,1,1) # Background color white
   renderer.ResetCamera()

   #Render and interact
   renderWindow.Render()

   renderWindowInteractor.Start()


if __name__ == '__main__':
   main()
