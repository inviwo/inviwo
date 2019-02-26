/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmTesting.h"
#include "gdcmImageHelper.h"
#include "gdcmImageReader.h"
#include "gdcmImageWriter.h"
#include "gdcmMediaStorage.h"
#include "gdcmDataSet.h"
#include "gdcmAttribute.h"
#include "gdcmDirectionCosines.h"

/*
 * Let's check this is easy with GDCM to handle the new
 * Real World Value Mapping Sequence mechanism
 */
int TestImageHelper2(int, char *[])
{
  static const char *filenames[] = {
    "D_CLUNIE_MR2_JPLL.dcm",
    "D_CLUNIE_MR2_JPLY.dcm",
    "D_CLUNIE_MR2_RLE.dcm",
    "D_CLUNIE_MR4_JPLL.dcm",
    "D_CLUNIE_MR4_JPLY.dcm",
    "D_CLUNIE_MR4_RLE.dcm",
    "MR-MONO2-12-shoulder.dcm",
    "MR_Philips-Intera_BreaksNOSHADOW.dcm",
    "MR_Philips_Intera_No_PrivateSequenceImplicitVR.dcm",
    "MR_Philips_Intera_PrivateSequenceExplicitVR_in_SQ_2001_e05f_item_wrong_lgt_use_NOSHADOWSEQ.dcm",
    "MR_Philips_Intera_SwitchIndianess_noLgtSQItem_in_trueLgtSeq.dcm",
    "PHILIPS_GDCM12xBug.dcm",
    "PHILIPS_GDCM12xBug2.dcm",
    "PHILIPS_Gyroscan-12-Jpeg_Extended_Process_2_4.dcm",
    "PHILIPS_Gyroscan-12-MONO2-Jpeg_Lossless.dcm",
    "PHILIPS_Gyroscan-8-MONO2-Odd_Sequence.dcm",
    "PHILIPS_Intera-16-MONO2-Uncompress.dcm",
    "PhilipsInteraSeqTermInvLen.dcm",
    "SIEMENS_MOSAIC_12BitsStored-16BitsJPEG.dcm",
    "THERALYS-12-MONO2-Uncompressed-Even_Length_Tag.dcm",
    "gdcm-MR-PHILIPS-16-Multi-Seq.dcm",
    "gdcm-MR-PHILIPS-16-NonSquarePixels.dcm",
    "PHILIPS_Gyroscan-12-Jpeg_Extended_Process_2_4.dcm", // need PVRG option
  };

  const unsigned int nfiles = sizeof(filenames)/sizeof(*filenames);
  const char *root = gdcm::Testing::GetDataRoot();
  if( !root || !*root )
    {
    std::cerr << "root is not defiend" << std::endl;
    return 1;
    }
  std::string sroot = root;
  sroot += "/";

  gdcm::Trace::WarningOff();
  for(unsigned int i = 0; i < nfiles; ++i)
  {
    const char * filename = filenames[i];
    std::string fullpath = sroot + filename;

    gdcm::ImageHelper::SetForceRescaleInterceptSlope(true);
    gdcm::ImageReader r;
    r.SetFileName( fullpath.c_str() );
    if( !r.Read() )
    {
      return 1;
    }
    gdcm::ImageHelper::SetForceRescaleInterceptSlope(false);
    gdcm::Image & img = r.GetImage();
    //std::cout << img.GetIntercept() << std::endl;
    //std::cout << img.GetSlope() << std::endl;
    // Create directory first:
    const char subdir[] = "TestImageHelper2";
    std::string tmpdir = gdcm::Testing::GetTempDirectory( subdir );
    if( !gdcm::System::FileIsDirectory( tmpdir.c_str() ) )
    {
      gdcm::System::MakeDirectory( tmpdir.c_str() );
      //return 1;
    }
    std::string outfilename = gdcm::Testing::GetTempFilename( filename, subdir );

    gdcm::ImageWriter writer;
    writer.SetFileName( outfilename.c_str() );
    writer.SetFile( r.GetFile() );
    writer.SetImage( r.GetImage() );
    if( !writer.Write() )
    {
      std::cerr << "Failed to write: " << outfilename << std::endl;
      return 1;
    }

    gdcm::ImageReader r2;
    r2.SetFileName( outfilename.c_str() );
    if( !r2.Read() )
    {
      return 1;
    }
    gdcm::Image & img2 = r2.GetImage();
    if( img.GetIntercept() != img2.GetIntercept() )
    {
      std::cerr << img2.GetIntercept() << std::endl;
      return 1;
    }
    if( img.GetSlope() != img2.GetSlope() )
    {
    std::cout << img2.GetSlope() << std::endl;
    return 1;
    }
  }

  return 0;
}
