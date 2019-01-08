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

#include <modules/cimg/cimglayerwriter.h>
#include <modules/cimg/cimgutils.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

CImgLayerWriter::CImgLayerWriter() : DataWriterType<Layer>() {
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
    addExtension(FileExtension("hdr", "Analyze 7.5"));
    addExtension(FileExtension("raw", "RAW"));
}

CImgLayerWriter* CImgLayerWriter::clone() const { return new CImgLayerWriter(*this); }

void CImgLayerWriter::writeData(const Layer* data, const std::string filePath) const {
    cimgutil::saveLayer(filePath, data);
}

std::unique_ptr<std::vector<unsigned char>> CImgLayerWriter::writeDataToBuffer(
    const Layer* data, const std::string& fileExtension) const {
    return cimgutil::saveLayerToBuffer(fileExtension, data);
}

bool CImgLayerWriter::writeDataToRepresentation(const repr* src, repr* dst) const {
    const LayerRAM* source = dynamic_cast<const LayerRAM*>(src);
    LayerRAM* target = dynamic_cast<LayerRAM*>(dst);

    if (!source || !target) {
        LogError("Target representation missing.");
        return false;
    }

    return cimgutil::rescaleLayerRamToLayerRam(source, target);
}

}  // namespace inviwo
