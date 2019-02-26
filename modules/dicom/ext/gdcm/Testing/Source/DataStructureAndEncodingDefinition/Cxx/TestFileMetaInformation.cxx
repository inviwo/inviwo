/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmFileMetaInformation.h"

#include "gdcmReader.h"
#include "gdcmTesting.h"

int TestFileMetaInformation(int, char *[])
{
  std::string dataroot = gdcm::Testing::GetDataRoot();
  std::string filename = dataroot + "/012345.002.050.dcm";

  gdcm::Reader reader;
  reader.SetFileName(filename.c_str());
  if ( !reader.Read() )
    {
    std::cerr << "Failed to read: " << filename << std::endl;
    return 1;
    }

  const gdcm::File& fi = reader.GetFile();
  const gdcm::FileMetaInformation& hd = fi.GetHeader();

  std::stringstream ss;
  hd.Write(ss);
  gdcm::FileMetaInformation hd2;

  // FAIL
  //hd2.Read(ss);
  
  // WORKS
  //ss.ignore(132);
  //hd2.Read(ss);

  return 0;
}
