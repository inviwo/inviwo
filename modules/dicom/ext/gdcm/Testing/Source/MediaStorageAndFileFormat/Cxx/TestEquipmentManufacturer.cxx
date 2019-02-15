/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmEquipmentManufacturer.h"
#include "gdcmReader.h"
#include "gdcmFilename.h"

#include "gdcmTesting.h"

static int TestEquipmentManufacturerFunc(const char* filename, bool verbose = false)
{
  if( verbose )
    std::cerr << "Reading: " << filename << std::endl;
  gdcm::Reader reader;
  reader.SetFileName( filename );
  if ( !reader.Read() )
    {
    std::cerr << "TestReadError: Failed to read: " << filename << std::endl;
    return 1;
    }

  const gdcm::File & file = reader.GetFile();
  gdcm::MediaStorage ms;
  ms.SetFromFile( file );
  if(ms == gdcm::MediaStorage::MediaStorageDirectoryStorage) return 0; // skip DICOMDIR

  gdcm::Filename fn( filename );
  const char *name = fn.GetName();
  // Special handling:
  if( strcmp(name, "OsirixFake16BitsStoredFakeSpacing.dcm" ) == 0
  || strcmp(name, "simpleImageWithIcon.dcm" ) == 0
  || strcmp(name, "rle16sti.dcm" ) == 0
  || strcmp(name, "rle16loo.dcm" ) == 0
  || strcmp(name, "gdcm-CR-DCMTK-16-NonSamplePerPix.dcm" ) == 0
  || strcmp(name, "XA-MONO2-8-12x-catheter.dcm" ) == 0
  || strcmp(name, "US-RGB-8-esopecho.dcm" ) == 0
  || strcmp(name, "US-PAL-8-10x-echo.dcm" ) == 0
  || strcmp(name, "US-MONO2-8-8x-execho.dcm" ) == 0
  || strcmp(name, "THERALYS-12-MONO2-Uncompressed-Even_Length_Tag.dcm" ) == 0
  || strcmp(name, "SignedShortLosslessBug.dcm" ) == 0
  || strcmp(name, "SC16BitsAllocated_8BitsStoredJPEG.dcm" ) == 0
  || strcmp(name, "SC16BitsAllocated_8BitsStoredJ2K.dcm" ) == 0
  || strcmp(name, "RadBWLossLess.dcm" ) == 0
  || strcmp(name, "RLEDebianBug816607Orig.dcm" ) == 0
  || strcmp(name, "DermaColorLossLess.dcm" ) == 0
  || strcmp(name, "00191113.dcm" ) == 0
  || strcmp(name, "test.acr" ) == 0
  || strcmp(name, "LIBIDO-8-ACR_NEMA-Lena_128_128.acr" ) == 0
  || strcmp(name, "gdcm-ACR-LibIDO.acr" ) == 0
  || strcmp(name, "libido1.0-vol.acr" ) == 0
  || strcmp(name, "DCMTK_JPEGExt_12Bits.dcm" ) == 0
  || strcmp(name, "DMCPACS_ExplicitImplicit_BogusIOP.dcm" ) == 0
  || strcmp(name, "DX_J2K_0Padding.dcm" ) == 0
  || strcmp(name, "GDCMJ2K_TextGBR.dcm" ) == 0
  || strcmp(name, "ITK_GDCM124_MultiframeSecondaryCaptureInvalid.dcm" ) == 0
  || strcmp(name, "JPEGLS_CharLS_10742.dcm" ) == 0
  || strcmp(name, "JPEGLosslessYBR_FULL_422.dcm" ) == 0
  || strcmp(name, "LIBIDO-24-ACR_NEMA-Rectangle.dcm" ) == 0
  || strcmp(name, "MR-Brucker-CineTagging-NonSquarePixels.dcm" ) == 0
  || strcmp(name, "MR16BitsAllocated_8BitsStored.dcm" ) == 0
  || strcmp(name, "NM-MONO2-16-13x-heart.dcm" ) == 0
  || strcmp(name, "NM-PAL-16-PixRep1.dcm" ) == 0
  || strcmp(name, "NM_Kakadu44_SOTmarkerincons.dcm" ) == 0
  || strcmp(name, "PICKER-16-MONO2-No_DicomV3_Preamble.dcm" ) == 0
  || strcmp(name, "OT-PAL-8-face.dcm" ) == 0
  || strcmp(name, "TG18-CH-2k-01.dcm" ) == 0 // wotsit ?
  || strncmp(name, "D_CLUNIE", 8) == 0 // D_CLUNIE*
  || strncmp(name, "LEADTOOLS_FLOWERS", 17) == 0 // LEADTOOLS_FLOWERS*
  || strncmp(name, "JDDICOM", 7) == 0 // JDDICOM*
  || strncmp(name, "JPEGNote", 8) == 0 // JPEGNote*
  || strncmp(name, "KODAK", 5) == 0 // KODAK*
  || strncmp(name, "DICOMDIR", 8) == 0 // DICOMDIR*
  || strncmp(name, "dicomdir", 8) == 0 // dicomdir*
  )
    {
    if( verbose )
      std::cout << "skip: " << filename << std::endl;
    return 0;
    }

  const gdcm::DataSet & ds = file.GetDataSet();
  gdcm::EquipmentManufacturer::Type manufacturer = gdcm::EquipmentManufacturer::Compute( ds );
  if( verbose )
    {
    std::cout << "Found: " << manufacturer << std::endl;
    }
  if( manufacturer == gdcm::EquipmentManufacturer::UNKNOWN )
    {
    std::cerr << "Unknown: " << filename << std::endl;
    return 1;
    }


  return 0;
}


int TestEquipmentManufacturer(int argc, char *argv[])
{
  if( argc == 2 )
    {
    const char *filename = argv[1];
    return TestEquipmentManufacturerFunc(filename, true);
    }

  // else
  // First of get rid of warning/debug message
  gdcm::Trace::DebugOff();
  gdcm::Trace::WarningOff();
  int r = 0, i = 0;
  const char *filename;
  const char * const *filenames = gdcm::Testing::GetFileNames();
  while( (filename = filenames[i]) )
    {
    r += TestEquipmentManufacturerFunc(filename);
    ++i;
    }

  return r;
}
