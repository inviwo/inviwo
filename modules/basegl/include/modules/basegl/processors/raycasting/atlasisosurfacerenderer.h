/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>

//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/imageport.h>
//#include <modules/opengl/shader/shader.h>

#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/raycastingproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/selectioncolorproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.AtlasIsosurfaceRenderer, Atlas Isosurface Renderer}
 * ![](org.inviwo.AtlasIsosurfaceRenderer.png?classIdentifier=org.inviwo.AtlasIsosurfaceRenderer)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __Atlas volume__ Atlas volume in range [0,3].
 *   * __Entry points__ Entry points.
 *   * __Exit points__ Exit points.
 *
 * ### Outports
 *   * __Rendered image__ Image rendering.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_BASEGL_API AtlasIsosurfaceRenderer : public Processor {
public:
    AtlasIsosurfaceRenderer();
    virtual ~AtlasIsosurfaceRenderer() override = default;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    void raycast(const Volume& volume);
    Shader shader_;


private:
    VolumeInport volumeInport_;
    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageOutport outport_;
    SelectionColorProperty showHighlighted_;
    SelectionColorProperty showSelected_;
    SelectionColorProperty showFiltered_;
    FloatProperty sampleRate_;
    Volume volume_;
    float sampleRateValue_;
};

}  // namespace inviwo
