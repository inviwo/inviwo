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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/shader/shader.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <modules/opengl/buffer/framebufferobject.h>

namespace inviwo {
enum class ReductionOperator { Min = 0, Max = 1, Sum = 2, None = 3};

/** \class GPUReduction
 *
 * GL implementation of add, min, and max reductions for 1, 2, and 3D textures.
 */
class IVW_MODULE_BASEGL_API GPUReduction {
public:
    template <typename Callback>
    GPUReduction(Callback C) : GPUReduction() {
        shader_.onReload(C);
    }

    GPUReduction();

    virtual ~GPUReduction() = default;

    std::shared_ptr<Volume> reduce(std::shared_ptr<const Volume> volume,
                                   ReductionOperator op);
    dvec4 reduce_v(std::shared_ptr<const Volume> volume,
                   ReductionOperator op);

    void setReductionOperator(ReductionOperator op);

protected:
    Shader shader_;
    FrameBufferObject fbo_;
    ReductionOperator activeReductionOperator_;

private:
    static void gpuDispatch(GLuint x, GLuint y, GLuint z);
};

}  // namespace inviwo
