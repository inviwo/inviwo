/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo {

VolumeDisk2RAMConverter::VolumeDisk2RAMConverter() : RepresentationConverterType<VolumeRAM>() {}

VolumeDisk2RAMConverter::~VolumeDisk2RAMConverter() {}

DataRepresentation* VolumeDisk2RAMConverter::createFrom(const DataRepresentation* source) {
    detail::VolumeDisk2RAMDispatcher disp;
    return source->getDataFormat()->dispatch(disp, source);
}

void VolumeDisk2RAMConverter::update(const DataRepresentation* source,
                                     DataRepresentation* destination) {
    const VolumeDisk* volumeSrc = static_cast<const VolumeDisk*>(source);
    VolumeRAM* volumeDst = static_cast<VolumeRAM*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions())
        volumeDst->setDimensions(volumeSrc->getDimensions());

    volumeSrc->readDataInto(volumeDst->getData());
}

}  // namespace
