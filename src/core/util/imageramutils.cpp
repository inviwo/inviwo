/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <inviwo/core/util/imageramutils.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

namespace util {

std::shared_ptr<Image> readImageFromDisk(std::string filename) {
    auto app = InviwoApplication::getPtr();
    auto factory = app->getDataReaderFactory();
    auto ext = filesystem::getFileExtension(filename);
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(ext)) {
        auto outLayer = reader->readData(filename);
        auto ram = outLayer->getRepresentation<LayerRAM>();
        outLayer->setDataFormat(ram->getDataFormat());

        auto img = std::make_shared<Image>(outLayer);
        img->getRepresentation<ImageRAM>();
        return img;
    } else {
        LogErrorCustom("util::readImageFromDisk", "Failed to read handle image");
        return nullptr;
    }
}
}  // namespace util

}  // namespace inviwo
