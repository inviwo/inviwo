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

#include <modules/opencl/volume/volumeclconverter.h>
#include <modules/opencl/volume/volumeclglconverter.h>
#include <modules/opencl/syncclgl.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <modules/opencl/inviwoopencl.h>

namespace inviwo {

std::shared_ptr<VolumeRAM> VolumeCLGL2RAMConverter::createFrom(
    std::shared_ptr<const VolumeCLGL> volumeCLGL) const {
    const size3_t dimensions{volumeCLGL->getDimensions()};
    auto destination = createVolumeRAM(dimensions, volumeCLGL->getDataFormat());

    if (destination) {
        volumeCLGL->getTexture()->download(destination->getData());
        // const cl::CommandQueue& queue = OpenCL::getPtr()->getQueue();
        // queue.enqueueReadVolume(volumeCL->get(), true, glm::size3_t(0), glm::size3_t(dimensions,
        // 1), 0, 0, volumeRAM->getData());
    } else {
        LogError("Invalid conversion or not implemented");
    }

    return destination;
}

void VolumeCLGL2RAMConverter::update(std::shared_ptr<const VolumeCLGL> volumeSrc,
                                     std::shared_ptr<VolumeRAM> volumeDst) const {
    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    volumeSrc->getTexture()->download(volumeDst->getData());

    if (volumeDst->hasHistograms()) volumeDst->getHistograms()->setValid(false);
}

std::shared_ptr<VolumeCLGL> VolumeGL2CLGLConverter::createFrom(
    std::shared_ptr<const VolumeGL> volumeGL) const {
    return std::make_shared<VolumeCLGL>(volumeGL->getDimensions(), volumeGL->getDataFormat(),
                                        volumeGL->getTexture());
}

void VolumeGL2CLGLConverter::update(std::shared_ptr<const VolumeGL> volumeSrc,
                                    std::shared_ptr<VolumeCLGL> volumeDst) const {
    // Do nothing since they are sharing data
    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }
}

std::shared_ptr<VolumeCL> VolumeCLGL2CLConverter::createFrom(
    std::shared_ptr<const VolumeCLGL> volumeCLGL) const {
#ifdef IVW_DEBUG
    LogWarn("Performance warning: Use shared CLGL representation instead of CL ");
#endif
    const size3_t dimensions{volumeCLGL->getDimensions()};
    auto destination = std::make_shared<VolumeCL>(dimensions, volumeCLGL->getDataFormat());
    {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(volumeCLGL.get());
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyImage(volumeCLGL->get(), destination->get(),
                                                      glm::size3_t(0), glm::size3_t(0),
                                                      glm::size3_t(dimensions));
    }
    return destination;
}

void VolumeCLGL2CLConverter::update(std::shared_ptr<const VolumeCLGL> volumeSrc,
                                    std::shared_ptr<VolumeCL> volumeDst) const {
    if (volumeSrc->getDimensions() != volumeDst->getDimensions()) {
        volumeDst->setDimensions(volumeSrc->getDimensions());
    }

    {
        SyncCLGL glSync;
        glSync.addToAquireGLObjectList(volumeSrc.get());
        glSync.aquireAllObjects();
        OpenCL::getPtr()->getQueue().enqueueCopyImage(volumeSrc->get(), volumeDst->get(),
                                                      glm::size3_t(0), glm::size3_t(0),
                                                      glm::size3_t(volumeSrc->getDimensions()));
    }
}

std::shared_ptr<VolumeGL> VolumeCLGL2GLConverter::createFrom(
    std::shared_ptr<const VolumeCLGL> src) const {
    return std::make_shared<VolumeGL>(src->getTexture(), src->getDataFormat());
}

void VolumeCLGL2GLConverter::update(std::shared_ptr<const VolumeCLGL> source,
                                    std::shared_ptr<VolumeGL> destination) const {
    // Do nothing since they share data
}

}  // namespace
