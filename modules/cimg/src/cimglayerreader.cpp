/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#include <modules/cimg/cimglayerreader.h>
#include <modules/cimg/cimgutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

CImgLayerReader::CImgLayerReader() : DataReaderType<Layer>() {
#ifdef cimg_use_jpeg
    addExtension(FileExtension("jpg", "Joint Photographic Experts Group"));
    addExtension(FileExtension("jpeg", "Joint Photographic Experts Group"));
#endif
#ifdef cimg_use_tiff
    addExtension(FileExtension("tif", "Tagged Image File Format"));
    addExtension(FileExtension("tiff", "Tagged Image File Format"));
#endif
    addExtension(FileExtension("bmp", "Windows bitmap"));
#ifdef cimg_use_openexr
    addExtension(FileExtension("exr", "OpenEXR"));
#endif
    addExtension(FileExtension("raw", "RAW"));
}

CImgLayerReader* CImgLayerReader::clone() const { return new CImgLayerReader(*this); }

std::shared_ptr<Layer> CImgLayerReader::readData(const std::string& filePath) {
    if (!filesystem::fileExists(filePath)) {
        throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
    }

    auto layerDisk = std::make_shared<LayerDisk>(filePath);
    layerDisk->setLoader(new CImgLayerRAMLoader(layerDisk.get()));
    auto layer = std::make_shared<Layer>(layerDisk);
    return layer;
}

CImgLayerRAMLoader::CImgLayerRAMLoader(LayerDisk* layerDisk) : layerDisk_(layerDisk) {}

CImgLayerRAMLoader* CImgLayerRAMLoader::clone() const { return new CImgLayerRAMLoader(*this); }

std::shared_ptr<LayerRepresentation> CImgLayerRAMLoader::createRepresentation() const {
    void* data = nullptr;

    uvec2 dimensions = layerDisk_->getDimensions();
    DataFormatId formatId = DataFormatId::NotSpecialized;

    std::string fileName = layerDisk_->getSourceFile();

    if (!filesystem::fileExists(fileName)) {
        std::string newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IvwContext);
        }
    }

    if (dimensions != uvec2(0)) {
        // Load and rescale to input dimensions
        data = cimgutil::loadLayerData(nullptr, fileName, dimensions, formatId, true);
    } else {
        // Load to original dimensions
        data = cimgutil::loadLayerData(nullptr, fileName, dimensions, formatId, false);
        layerDisk_->setDimensions(dimensions);
    }

    layerDisk_->updateDataFormat(DataFormatBase::get(formatId));
    updateSwizzleMask(layerDisk_);

    return dispatching::dispatch<std::shared_ptr<LayerRepresentation>, dispatching::filter::All>(
        formatId, *this, data);
}

void CImgLayerRAMLoader::updateRepresentation(std::shared_ptr<LayerRepresentation> dest) const {
    auto layerDst = std::static_pointer_cast<LayerRAM>(dest);

    if (layerDisk_->getDimensions() != layerDst->getDimensions()) {
        layerDst->setDimensions(layerDisk_->getDimensions());
    }

    uvec2 dimensions = layerDisk_->getDimensions();
    DataFormatId formatId = DataFormatId::NotSpecialized;

    std::string fileName = layerDisk_->getSourceFile();

    if (!filesystem::fileExists(fileName)) {
        std::string newPath = filesystem::addBasePath(fileName);

        if (filesystem::fileExists(newPath)) {
            fileName = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + fileName, IvwContext);
        }
    }

    if (dimensions != uvec2(0)) {
        // Load and rescale to input dimensions
        cimgutil::loadLayerData(layerDst->getData(), fileName, dimensions, formatId, true);
    } else {
        // Load to original dimensions
        cimgutil::loadLayerData(layerDst->getData(), fileName, dimensions, formatId, false);
        layerDisk_->setDimensions(dimensions);
    }

    layerDisk_->updateDataFormat(DataFormatBase::get(formatId));
    updateSwizzleMask(layerDisk_);
}

void CImgLayerRAMLoader::updateSwizzleMask(LayerDisk* layerDisk) {
    auto swizzleMask = [](std::size_t numComponents) {
        switch (numComponents) {
            case 1:
                return swizzlemasks::luminance;
            case 2:
                return swizzlemasks::luminanceAlpha;
            case 3:
                return swizzlemasks::rgb;
            case 4:
            default:
                return swizzlemasks::rgba;
        }
    };

    layerDisk->setSwizzleMask(swizzleMask(layerDisk->getDataFormat()->getComponents()));
}

}  // namespace inviwo
