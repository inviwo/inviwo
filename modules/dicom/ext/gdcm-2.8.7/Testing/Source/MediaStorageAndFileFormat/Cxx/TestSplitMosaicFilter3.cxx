/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmSplitMosaicFilter.h"
#include "gdcmTesting.h"
#include "gdcmSystem.h"
#include "gdcmReader.h"
#include "gdcmAnonymizer.h"
#include "gdcmPrivateTag.h"
#include "gdcmCSAHeader.h"
#include "gdcmElement.h"
#include "gdcmDirectionCosines.h"

int TestSplitMosaicFilter3(int , char *[])
{
  const char *directory = gdcm::Testing::GetDataRoot();
  std::string filename = std::string(directory) + "/SIEMENS_MOSAIC_12BitsStored-16BitsJPEG.dcm";
  gdcm::SplitMosaicFilter s;
  if( !gdcm::System::FileExists(filename.c_str()) )
    {
    return 1;
    }

  gdcm::Reader reader;
  reader.SetFileName( filename.c_str() );
  if( !reader.Read() )
    {
    std::cerr << "could not read: " << filename << std::endl;
    return 1;
    }

  gdcm::SplitMosaicFilter filter;
  filter.SetFile( reader.GetFile() );
  bool inverted;
  double slicenormal[3];
  bool b = filter.ComputeMOSAICSliceNormal( slicenormal, inverted );
  if( !b )
  {
    std::cerr << "Could not ComputeMOSAICSliceNormal: " << filename << std::endl;
    return 1;
  }

  const double refnor[3] = { -0.03737130908,-0.314588168,0.9484923141 };
  const double eps = 1e-6;
  gdcm::DirectionCosines dc;
  const double dot = dc.Dot( slicenormal, refnor );
  if( std::fabs( 1.0 - dot ) > eps )
  {
    std::cerr << "Invalid ComputeMOSAICSliceNormal: " << filename << std::endl;
    return 1;
  }

  double slicepos[3];
  b = filter.ComputeMOSAICSlicePosition( slicepos, inverted );
  if( !b )
  {
    std::cerr << "Could not ComputeMOSAICSlicePosition: " << filename << std::endl;
    return 1;
  }

  const double refpos[3] = { -10.48860023,-7.82515782,-28.87523447 };

  for( int i = 0; i < 3; ++i ) 
  {
    if( std::fabs( refpos[i] - slicepos[i] ) > eps )
    {
      std::cerr << "Invalid ComputeMOSAICSlicePosition: " << filename << std::endl;
      return 1;
    }
  }

  gdcm::CSAHeader csa;
  gdcm::DataSet & ds = reader.GetFile().GetDataSet();
  gdcm::MrProtocol mrprot;
  if( !csa.GetMrProtocol(ds, mrprot))
  {
    std::cerr << "No MrProtocol" << filename << std::endl;
    return 1;
  }

  gdcm::MrProtocol::SliceArray sa;
  b = mrprot.GetSliceArray(sa);
  if( !b || sa.Slices.size() != 18 )
  {
    std::cerr << "Size" << filename << std::endl;
    return 1;
  }

  gdcm::MrProtocol::Slice & slice0 = sa.Slices[0];
  gdcm::MrProtocol::Vector3 & p0 = slice0.Position;
  double pos0[3];
  pos0[0] = p0.dSag;
  pos0[1] = p0.dCor;
  pos0[2] = p0.dTra;
  for( int i = 0; i < 3; ++i ) 
  {
    if( std::fabs( refpos[i] - pos0[i] ) > eps )
    {
      std::cerr << "Invalid slice0: " << filename << std::endl;
      return 1;
    }
  }

  gdcm::MrProtocol::Slice & slice1 = sa.Slices[1];
  gdcm::MrProtocol::Vector3 & p1 = slice1.Position;
  double pos1[3];
  pos1[0] = p1.dSag;
  pos1[1] = p1.dCor;
  pos1[2] = p1.dTra;
  double altnor[3];
  for( int i = 0; i < 3; ++i ) 
  {
    altnor[i] = pos1[i] - pos0[i];
  }
  dc.Normalize( altnor );
  const double dot2 = dc.Dot( altnor, refnor );
  if( std::fabs( 1.0 - dot2 ) > eps )
  {
    std::cerr << "Incompatible alt " << filename << std::endl;
    return 1;
  }

  for( int k = 0; k < 18; ++k )
  {
    gdcm::MrProtocol::Slice & slice = sa.Slices[k];
    gdcm::MrProtocol::Vector3 & nor = slice.Normal;
    double normal[3];
    normal[0] = nor.dSag;
    normal[1] = nor.dCor;
    normal[2] = nor.dTra;
    for( int i = 0; i < 3; ++i ) 
    {
      if( std::fabs( refnor[i] - normal[i] ) > eps )
      {
        std::cerr << "Invalid normal: " << filename << std::endl;
        return 1;
      }
    }
  }

 
  gdcm::Anonymizer ano;
  ano.SetFile( reader.GetFile() );
  const gdcm::PrivateTag &t1 = csa.GetCSAImageHeaderInfoTag();
  ano.Remove( t1 );

  unsigned int modims[3];
  b = filter.ComputeMOSAICDimensions( modims );
  if(! b )
  {
    std::cerr << "ComputeMOSAICDimensions: " << filename << std::endl;
    return 1;
  }

  const unsigned int ref[3] = { 64u, 64u, 18u };
  if( modims[0] != ref[0] || modims[1] != ref[1] || modims[2] != ref[2] )
  {
    std::cerr << "Invalid ComputeMOSAICDimensions " << filename << std::endl;
    return 1;
  }

  return 0;
}
