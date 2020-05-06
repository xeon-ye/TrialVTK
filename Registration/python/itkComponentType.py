import itk

imageMetaIO = itk.MetaImageIO.New()
imageMetaIO.SetFileName('/home/jmh/bkmedical/data/CT/CT-Abdomen.mhd')
imageMetaIO.ReadImageInformation()
imageType = imageMetaIO.GetComponentType()
print(imageType)
