/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/meshrenderinggl/datastructures/rasterization.h>
#include <modules/meshrenderinggl/ports/rasterizationport.h>
#include <modules/base/properties/transformlistproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.TransformRasterization, Transform Rasterization}
 * ![](org.inviwo.TransformRasterization.png?classIdentifier=org.inviwo.TransformRasterization)
 * Add a transformation on a rasterization object.
 *
 * ### Inports
 *   * __input__ Input rasterization object to be rendered by a rasterization renderer
 *
 * ### Outports
 *   * __output__ Wrapper around the input rasterization with a transform applied on rasterize call
 *
 * ### Properties
 *   * __Mesh Transform__ Additional world transform applied to all input meshes
 */

/**
 * \class TransformRasterization
 * \brief Applies an additional transform on a given rasterization object.
 */
class IVW_MODULE_MESHRENDERINGGL_API TransformRasterization : public Processor {
public:
    TransformRasterization();
    virtual ~TransformRasterization() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    RasterizationInport inport_;
    RasterizationOutport outport_;
    TransformListProperty transformSetting_;
};

}  // namespace inviwo
