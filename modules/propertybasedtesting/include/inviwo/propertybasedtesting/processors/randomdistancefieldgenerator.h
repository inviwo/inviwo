/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.RandomDistanceFieldGenerator, Random Volume Generator}
 * ![](org.inviwo.RandomDistanceFieldGenerator.png?classIdentifier=org.inviwo.RandomDistanceFieldGenerator)
 *
 * Generate a pseudo-random volume by generating a given number of points from a
 * given seed. The value of each voxel is the distance to the closest point.
 *
 * ### Outports
 *   * __outport__ The generated volume.
 *
 * ### Properties
 *   * __Seed__ The seed used for the random number generator.
 *   * __Number of Points__ The number of points to use for the volume
 *   generation.
 *   * __Resolution__ The resolution of the generated volume. If the resolution
 *   is set to n, there are n^3 voxels in the volume.
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API RandomDistanceFieldGenerator : public Processor {
public:
    RandomDistanceFieldGenerator();
    virtual ~RandomDistanceFieldGenerator() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeOutport outport_;
    IntProperty seed_;
    IntProperty numPoints_;
    IntProperty resolution_;
};

}  // namespace inviwo
