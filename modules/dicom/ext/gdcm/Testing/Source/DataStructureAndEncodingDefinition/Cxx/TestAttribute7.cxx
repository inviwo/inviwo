/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmAttribute.h"

int TestAttribute7(int, char *[])
{
{
  const char bytes[] = "\030\000e\020";
  gdcm::DataElement de( gdcm::Tag(0x28,0x9)  );
  de.SetVR( gdcm::VR::INVALID );
  de.SetByteValue( bytes, 4 );

  gdcm::Attribute<0x28,0x9, gdcm::VR::AT, gdcm::VM::VM1 > at;
  at.SetFromDataElement( de );
  std::cout << at.GetValue() << std::endl;

  gdcm::Attribute<0x3004, 0x0014> tissue;
  //std::cout << tissue.GetVR() << std::endl;
  if( tissue.GetVR() != gdcm::VR::CS ) return 1;
}

{
  gdcm::Attribute<0x8,0x8> imagetype;
  imagetype.SetNumberOfValues(0);
  if( imagetype.GetNumberOfValues() != 0 ) return 1;

  const char bytes[] = "ORIGINAL\\PRIMARY";
  gdcm::DataElement de( gdcm::Tag(0x8,0x8)  );
  de.SetVR( gdcm::VR::INVALID );
  de.SetByteValue( bytes, (uint32_t)strlen(bytes) );
  gdcm::DataSet ds;
  imagetype.SetFromDataSet( ds );
  if( imagetype.GetNumberOfValues() != 0 ) return 1;
  ds.Insert( de );
  imagetype.SetFromDataSet( ds );
  if( imagetype.GetNumberOfValues() != 2 ) return 1;
  imagetype.SetNumberOfValues(0);
  if( imagetype.GetNumberOfValues() != 0 ) return 1;
  imagetype.SetFromDataSet( ds );
  if( imagetype.GetNumberOfValues() != 2 ) return 1;


}


  return 0;
}
