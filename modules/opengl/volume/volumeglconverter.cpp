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

#include "volumeglconverter.h"
#include <inviwo/core/datastructures/volume/volumerepresentation.h>

namespace inviwo {

VolumeRAM2GLConverter::VolumeRAM2GLConverter()
    : RepresentationConverterType<VolumeGL>()
{}

VolumeRAM2GLConverter::~VolumeRAM2GLConverter() {}

DataRepresentation* VolumeRAM2GLConverter::createFrom(const DataRepresentation* source) {
    const VolumeRAM* volumeRAM = static_cast<const VolumeRAM*>(source);
    VolumeGL* volume = new VolumeGL(volumeRAM->getDimensions(), volumeRAM->getDataFormat(), false);
    volume->getTexture()->initialize(volumeRAM->getData());
    return volume;
}

void VolumeRAM2GLConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeRAM* volumeSrc = static_cast<const VolumeRAM*>(source);
    VolumeGL* volumeDst = static_cast<VolumeGL*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeDst->getTexture()->upload(volumeSrc->getData());
}

VolumeGL2RAMConverter::VolumeGL2RAMConverter()
    : RepresentationConverterType<VolumeRAM>()
{}

VolumeGL2RAMConverter::~VolumeGL2RAMConverter() {}

DataRepresentation* VolumeGL2RAMConverter::createFrom(const DataRepresentation* source) {
    const VolumeGL* volumeGL = static_cast<const VolumeGL*>(source);
    VolumeRAM* volume = createVolumeRAM(volumeGL->getDimensions(), volumeGL->getDataFormat());

    if (volume) {
        volumeGL->getTexture()->download(volume->getData());
        return volume;
    } else {
        LogError("Cannot convert format from GL to RAM:" << volumeGL->getDataFormat()->getString());
    }

    return nullptr;
}

void VolumeGL2RAMConverter::update(const DataRepresentation* source, DataRepresentation* destination) {
    const VolumeGL* volumeSrc = static_cast<const VolumeGL*>(source);
    VolumeRAM* volumeDst = static_cast<VolumeRAM*>(destination);

    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeSrc->getTexture()->download(volumeDst->getData());

    if (volumeDst->hasNormalizedHistogram())
        volumeDst->getNormalizedHistogram()->setValid(false);
}

} // namespace
