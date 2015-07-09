/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_SURFACEEXTRACTION_H
#define IVW_SURFACEEXTRACTION_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <future>

namespace inviwo {
/** \docpage{org.inviwo.SurfaceExtraction, Surface Extraction}
 * ![](org.inviwo.SurfaceExtraction.png?classIdentifier=org.inviwo.SurfaceExtraction)
 *
 * ...
 *
 * ### Inports
 *   * __volume__ ...
 *
 * ### Outports
 *   * __mesh__ ...
 *
 * ### Properties
 *   * __ISO Value__ ...
 *   * __Method__ ...
 *   * __Triangle Color__ ...
 *
 */
class IVW_MODULE_BASE_API SurfaceExtraction : public Processor, public ProgressBarOwner {
public:
    InviwoProcessorInfo();
    
    SurfaceExtraction();
    virtual ~SurfaceExtraction();

    SurfaceExtraction(const SurfaceExtraction&) = delete;
    SurfaceExtraction& operator=(const SurfaceExtraction&) = delete;


protected:
    virtual void process();
    void setMinMax();
    void updateColors();

    struct task {
        task() = default;
        task(const task&) = delete;
        task& operator=(const task&) = delete;
        task(task&&);
        task& operator=(task&&);

        std::future<std::unique_ptr<Mesh>> result;
        float iso = 0.0f;
        vec4 color = vec4(0);
        float status = 0.0f;
    };

    DataInport<Volume, 0> volume_;
    DataOutport<std::vector<std::unique_ptr<Mesh>>> mesh_;

    FloatProperty isoValue_;
    OptionPropertyInt method_;
    CompositeProperty colors_;

    std::vector<task> result_;
};

}  // namespace

#endif  // IVW_SURFACEEXTRACTION_H
