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

#include <modules/cimg/cimglayerwriter.h>
#include <modules/cimg/cimgutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/image/layer.h>

namespace inviwo {

CImgLayerWriter::CImgLayerWriter() 
    : DataWriterType<Layer>() {
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
    addExtension(FileExtension("hdr", "Analyze 7.5"));
}

CImgLayerWriter::CImgLayerWriter(const CImgLayerWriter& rhs) : DataWriterType<Layer>(rhs) {}

CImgLayerWriter& CImgLayerWriter::operator=(const CImgLayerWriter& that) {
    if (this != &that)
        DataWriterType<Layer>::operator=(that);

    return *this;
}

CImgLayerWriter* CImgLayerWriter::clone() const {
    return new CImgLayerWriter(*this);
}

void CImgLayerWriter::writeData(const Layer* data, const std::string filePath) const {
    CImgUtils::saveLayer(filePath, data);
}

std::unique_ptr<std::vector<unsigned char>> CImgLayerWriter::writeDataToBuffer(const Layer* data, std::string& type) const {
    return CImgUtils::saveLayerToBuffer(type, data);
}

bool CImgLayerWriter::writeDataToRepresentation(const DataRepresentation* src, DataRepresentation* dst) const {
    const LayerRAM* source = dynamic_cast<const LayerRAM*>(src);
    LayerRAM* target = dynamic_cast<LayerRAM*>(dst);

    if (!source || !target) {
        return false;
        LogError("Target representation missing.");
    }

    if (!source->getData())
        return true;
    
    uvec2 dimensions = target->getDimensions();

    void* rawData = CImgUtils::rescaleLayerRAM(source, dimensions);

    if (!rawData)
        return false;

    target->setData(rawData, dimensions);

    return true;
}

} // namespace
