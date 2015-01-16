/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <modules/freeimage/freeimagereader.h>
#include <modules/freeimage/freeimageutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerdisk.h>

namespace inviwo {

FreeImageReader::FreeImageReader()
    : DataReaderType<Layer>() {
    addExtension(FileExtension("png", "Portable Network Graphics"));
    addExtension(FileExtension("jpg", "Joint Photographic Experts Group"));
    addExtension(FileExtension("jpeg", "Joint Photographic Experts Group"));
    addExtension(FileExtension("tiff", "Tagged Image File Format"));
    addExtension(FileExtension("tga", "Truevision TGA"));
    addExtension(FileExtension("bmp", "Windows bitmap"));
    addExtension(FileExtension("exr", "OpenEXR"));
    addExtension(FileExtension("hdr", "High dynamic range"));
}

FreeImageReader::FreeImageReader(const FreeImageReader& rhs)
    : DataReaderType<Layer>(rhs) {};

FreeImageReader& FreeImageReader::operator=(const FreeImageReader& that) {
    if (this != &that) {
        DataReaderType<Layer>::operator=(that);
    }

    return *this;
}

FreeImageReader* FreeImageReader::clone() const { return new FreeImageReader(*this); }

Layer* FreeImageReader::readMetaData(std::string filePath) {
    if (!filesystem::fileExists(filePath)) {
        std::string newPath = filesystem::addBasePath(filePath);

        if (filesystem::fileExists(newPath)) {
            filePath = newPath;
        } else {
            throw DataReaderException("Error could not find input file: " + filePath);
        }
    }

    Layer* layer = new Layer();

    LayerDisk* layerDisk = new LayerDisk(filePath);
    layerDisk->setDataReader(this);

    layer->addRepresentation(layerDisk);

    return layer;
}

void FreeImageReader::readDataInto(void* destination) const {
    LayerDisk* layerDisk = dynamic_cast<LayerDisk*>(owner_);
    if(layerDisk){
        uvec2 dimensions = layerDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        if (dimensions != uvec2(0))
            FreeImageUtils::loadImageToDataAndRescale(destination, layerDisk->getSourceFile(), dimensions, formatId);
        else{
            FreeImageUtils::loadImageToData(destination, layerDisk->getSourceFile(), dimensions, formatId);
            layerDisk->setDimensions(dimensions);
        }

        layerDisk->updateDataFormat(DataFormatBase::get(formatId));
    }
}

void* FreeImageReader::readData() const {
    void* data = NULL;

    LayerDisk* layerDisk = dynamic_cast<LayerDisk*>(owner_);
    if(layerDisk){
        uvec2 dimensions = layerDisk->getDimensions();
        DataFormatEnums::Id formatId = DataFormatEnums::NOT_SPECIALIZED;

        if (dimensions != uvec2(0))
            data = FreeImageUtils::loadImageToDataAndRescale(NULL, layerDisk->getSourceFile(), dimensions, formatId);
        else{
            data = FreeImageUtils::loadImageToData(NULL, layerDisk->getSourceFile(), dimensions, formatId);
            layerDisk->setDimensions(dimensions);
        }

        layerDisk->updateDataFormat(DataFormatBase::get(formatId));
    }

    return data;
}

}  // namespace
