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

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>
#include <modules/basegl/raycasting/raycastercomponent.h>

#include <inviwo/core/util/stdextensions.h>
#include <functional>
#include <string_view>
#include <tcb/span.hpp>

namespace inviwo {

namespace util {

constexpr auto bind_front = [](auto&& func, auto&& obj) constexpr {
    return [f = std::forward<decltype(func)>(func), o = std::forward<decltype(obj)>(obj)](
               auto&&... args) { return std::invoke(f, o, std::forward<decltype(args)>(args)...); };
};

}  // namespace util

/**
 * @brief Base class for volume raycasters.
 * Derived classes should register a set of RaycasterComponents to customize behavior
 */
class IVW_MODULE_BASEGL_API VolumeRaycasterBase : public Processor {
protected:
    VolumeRaycasterBase(std::string_view identifier = "", std::string_view displayName = "");
    VolumeRaycasterBase(const VolumeRaycasterBase&) = delete;
    VolumeRaycasterBase& operator=(const VolumeRaycasterBase&) = delete;
    virtual ~VolumeRaycasterBase();

    void registerComponents(util::span<RaycasterComponent*> comps);

    virtual void initializeResources() override;

    virtual void process() override;

    virtual void handleError(std::string_view action, std::string_view name) const;

    ImageInport entryPort_;
    ImageInport exitPort_;
    ImageOutport outport_;
    Shader shader_;
    std::vector<RaycasterComponent*> components_;
};

}  // namespace inviwo
