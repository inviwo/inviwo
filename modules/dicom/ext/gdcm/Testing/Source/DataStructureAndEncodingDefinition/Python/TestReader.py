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

import os,sys
import gdcm

def TestRead(filename, verbose = False):
  r = gdcm.Reader()
  r.SetFileName( filename )
  success = r.Read()
  #if verbose: print r.GetFile()
  if verbose: print(r.GetFile().GetDataSet())
  return success

if __name__ == "__main__":
  success = 0
  try:
    filename = os.sys.argv[1]
    success += TestRead( filename, True )
  except:
    # loop over all files:
    gdcm.Trace.DebugOff()
    gdcm.Trace.WarningOff()
    t = gdcm.Testing()
    nfiles = t.GetNumberOfFileNames()
    for i in range(0,nfiles):
      filename = t.GetFileName(i)
      success += TestRead( filename )


  # Test succeed ?
  sys.exit(success == 0)
