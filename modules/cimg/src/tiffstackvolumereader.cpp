/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/cimg/tiffstackvolumereader.h>

#include "tiffio.h" // Reading TIF Header before loading the whole File

#include <modules/cimg/cimgutils.h> // loading tif as volume

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

TiffStackVolumeReader::TiffStackVolumeReader() : DataReaderType<Volume>() {
	addExtension(FileExtension("tif", "Tagged Image File Format Stacks"));
	addExtension(FileExtension("tiff", "Tagged Image File Format Stacks"));
}

TiffStackVolumeReader* TiffStackVolumeReader::clone() const { return new TiffStackVolumeReader(*this); }

std::shared_ptr<Volume> TiffStackVolumeReader::readData(const std::string & filePath)
{
	if (!filesystem::fileExists(filePath)) {
		throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
	}

    TIFF* tif = TIFFOpen(filePath.c_str(), "r");

	uint32 x = 0, y = 0, z = 0;
    size3_t dimension = size3_t(x, y, z);
    const DataFormatBase* format = DataFormatBase::get();


	if(tif) {
        TIFFSetDirectory(tif, 0);

        //float xres,yres;
        uint16 samplesPerPixel = 1, bitsPerSample = 8, sampleFormat = 1;

        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
        TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
        //TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres);
        //TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres);

        if (sampleFormat != 1 || (bitsPerSample % 8) != 0) {
            throw DataReaderException("Error: internal tif format not supported: " + filePath, IvwContext);
        }

        switch (samplesPerPixel) {
        case (4):
            if (bitsPerSample == 8) {
                format = DataVec4UInt8::get();
            } else if (bitsPerSample == 16) {
                format = DataVec4UInt16::get();
            } else {
                format = DataVec4UInt32::get();
            }
            break;
        case (3):
            if (bitsPerSample == 8) {
                format = DataVec3UInt8::get();
            } else if (bitsPerSample == 16) {
                format = DataVec3UInt16::get();
            } else {
                format = DataVec3UInt32::get();
            }
            break;
        case (2):
            if (bitsPerSample == 8) {
                format = DataVec2UInt8::get();
            } else if (bitsPerSample == 16) {
                format = DataVec2UInt16::get();
            } else {
                format = DataVec2UInt32::get();
            }
            break;
        case (1):

            if (bitsPerSample == 8) {
                format = DataUInt8::get();
            } else if (bitsPerSample == 16) {
                format = DataUInt16::get();
            } else {
                format = DataUInt32::get();
            }
            break;
        default:
            format = DataVec3UInt32::get();
        }

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &x);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &y);

		// count the pictures
		do ++z; while (TIFFReadDirectory(tif));

		dimension = size3_t(x, y, z);
	}
	else {
		throw DataReaderException("Error could not open input file: " + filePath, IvwContext);
	}

    TIFFClose(tif);

	auto volume = std::make_shared<Volume>(dimension,format);

	auto volumeDisk = std::make_shared<VolumeDisk>(filePath,dimension, format);
	volume->setDataFormat(format);
;
	volumeDisk->setLoader(new TiffStackVolumeRAMLoader(volumeDisk.get()));
	volume->addRepresentation(volumeDisk);

	return volume;
}

TiffStackVolumeRAMLoader * TiffStackVolumeRAMLoader::clone() const {return new TiffStackVolumeRAMLoader(*this); }

std::shared_ptr<VolumeRepresentation> TiffStackVolumeRAMLoader::createRepresentation() const
{
	size3_t dimensions = volumeDisk_->getDimensions();


	std::string fileName = volumeDisk_->getSourceFile();

	if (!filesystem::fileExists(fileName)) {
		std::string newPath = filesystem::addBasePath(fileName);

		if (filesystem::fileExists(newPath)) {
			fileName = newPath;
		}
		else {
			throw DataReaderException("Error could not find input file: " + fileName, IvwContext);
		}
	}
	//formatId makes no sense because it is determined by file extension, tiff supports multiple formats
	DataFormatId formatId = volumeDisk_->getDataFormatId();
	auto data = cimgutil::loadTiffVolumeData(nullptr, fileName, dimensions, formatId);

	return dispatching::dispatch<std::shared_ptr<VolumeRepresentation>, dispatching::filter::All>(
		volumeDisk_->getDataFormat()->getId(), *this, data);
}

void TiffStackVolumeRAMLoader::updateRepresentation(std::shared_ptr<VolumeRepresentation> dest) const
{
	// TODO: create updateRepresentation.
    return;
}
}  // namespace inviwo
