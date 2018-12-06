############################################################################
#
#  Program: GDCM (Grassroots DICOM). A DICOM library
#
#  Copyright (c) 2006-2011 Mathieu Malaterre
#  All rights reserved.
#  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notice for more information.
#
############################################################################

"""
Usage:

 python DecompressImage.py gdcmData/012345.002.050.dcm decompress.dcm
"""

import gdcm
import sys

if __name__ == "__main__":

  file1 = sys.argv[1]
  file2 = sys.argv[2]

  r = gdcm.ImageReader()
  r.SetFileName( file1 )
  if not r.Read():
    sys.exit(1)

  # check GetFragment API:
  pd = r.GetFile().GetDataSet().GetDataElement(gdcm.Tag(0x7fe0, 0x0010))
  frags = pd.GetSequenceOfFragments();
  frags.GetFragment(0);

  ir = r.GetImage()
  w = gdcm.ImageWriter()
  image = w.GetImage()

  image.SetNumberOfDimensions( ir.GetNumberOfDimensions() );
  dims = ir.GetDimensions();
  print ir.GetDimension(0);
  print ir.GetDimension(1);
  print "Dims:",dims

  #  Just for fun:
  dircos =  ir.GetDirectionCosines()
  t = gdcm.Orientation.GetType(tuple(dircos))
  l = gdcm.Orientation.GetLabel(t)
  print "Orientation label:",l

  image.SetDimension(0, ir.GetDimension(0) );
  image.SetDimension(1, ir.GetDimension(1) );

  pixeltype = ir.GetPixelFormat();
  image.SetPixelFormat( pixeltype );

  pi = ir.GetPhotometricInterpretation();
  image.SetPhotometricInterpretation( pi );

  pixeldata = gdcm.DataElement( gdcm.Tag(0x7fe0,0x0010) )
  str1 = ir.GetBuffer()
  #print ir.GetBufferLength()
  pixeldata.SetByteValue( str1, gdcm.VL( len(str1) ) )
  image.SetDataElement( pixeldata )

  w.SetFileName( file2 )
  w.SetFile( r.GetFile() )
  w.SetImage( image )
  if not w.Write():
    sys.exit(1)
