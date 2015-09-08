/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerdisk.h>

namespace inviwo {

CImgLayerReader::CImgLayerReader()
    : DataReaderType<Layer>() {
    addExtension(FileExtension("raw", "RAW"));
#ifdef cimg_use_png
    addExtension(FileExtension("png", "Portable Network Graphics"));
#endif
#ifdef cimg_use_jpeg
    addExtension(FileExtension("jpg", "Joint Photographic Experts Group"));
    addExtension(FileExtension("jpeg", "Joint Photographic Experts Group"));
#endif
    addExtension(FileExtension("bmp", "Windows bitmap"));
#ifdef cimg_use_openexr
    addExtension(FileExtension("exr", "OpenEXR"));
#endif
}

CImgLayerReader::CImgLayerReader(const CImgLayerReader& rhs)
    : DataReaderType<Layer>(rhs) {};

CImgLayerReader& CImgLayerReader::operator=(const CImgLayerReader& that) {
    if (this != &that) {
        DataReaderType<Layer>::operator=(that);
    }

    return *this;
}

CImgLayerReader* CImgLayerReader::clone() const { return new CImgLayerReader(*this); }

Layer* CImgLayerReader::readMetaData(std::string filePath) {
    if (!filesystem::fileExists(filePath)) {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath)) {
            filePath = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
        }
    }

    Layer* layer = new Layer();

    LayerDisk* layerDisk = new LayerDisk(filePath);
    layerDisk->setDataReader(this->clone());

    layer->addRepresentation(layerDisk);

    return layer;
}

void CImgLayerReader::readDataInto(void* destination) const {
    LayerDisk* layerDisk = dynamic_cast<LayerDisk*>(owner_);
    if(layerDisk){
        uvec2 dimensions = layerDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        std::string filePath = layerDisk->getSourceFile();

        if (!filesystem::fileExists(filePath)) {
            std::string newPath = filesystem::addBasePath(filePath);

            if (filesystem::fileExists(newPath)) {
                filePath = newPath;
            }
            else {
                throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
            }
        }

        if (dimensions != uvec2(0)){
            // Load and rescale to input dimensions
            CImgUtils::loadLayerData(destination, filePath, dimensions, formatId, true);
        }
        else{
            // Load to original dimensions
            CImgUtils::loadLayerData(destination, filePath, dimensions, formatId, false);
            layerDisk->setDimensions(dimensions);
        }

        layerDisk->updateDataFormat(DataFormatBase::get(formatId));
    }
}

void* CImgLayerReader::readData() const {
    void* data = nullptr;

    LayerDisk* layerDisk = dynamic_cast<LayerDisk*>(owner_);
    if(layerDisk){
        uvec2 dimensions = layerDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        std::string filePath = layerDisk->getSourceFile();

        if (!filesystem::fileExists(filePath)) {
            std::string newPath = filesystem::addBasePath(filePath);

            if (filesystem::fileExists(newPath)) {
                filePath = newPath;
            }
            else {
                throw DataReaderException("Error could not find input file: " + filePath, IvwContext);
            }
        }

        if (dimensions != uvec2(0)){
            // Load and rescale to input dimensions
            data = CImgUtils::loadLayerData(nullptr, filePath, dimensions, formatId, true);
        }
        else{
            // Load to original dimensions
            data = CImgUtils::loadLayerData(nullptr, filePath, dimensions, formatId, false);
            layerDisk->setDimensions(dimensions);
        }

        layerDisk->updateDataFormat(DataFormatBase::get(formatId));
    }

    return data;
}

}  // namespace
