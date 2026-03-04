/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2026 Inviwo Foundation
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

#include <modules/basegl/processors/volumeprocessing/volumemerger.h>

#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/processors/processorstate.h>
#include <inviwo/core/processors/processortags.h>
#include <inviwo/core/util/formats.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderobject.h>
#include <modules/opengl/volume/volumeutils.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace inviwo {
class TextureUnitContainer;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeMerger::processorInfo_{
    "org.inviwo.VolumeMerger",  // Class identifier
    "Volume Merger",            // Display name
    "Volume Operation",         // Category
    CodeState::Stable,          // Code state
    "GL",                       // Tags
    R"(Merges up to four single-channel volumes into a single volume. If, for example,
    input volumes 1 and 4 are given, the output volume will have 2 channels where the
    first one contains volume 1 and the second one volume 4.)"_unindentHelp,
};
const ProcessorInfo& VolumeMerger::getProcessorInfo() const { return processorInfo_; }

VolumeMerger::VolumeMerger()
    : VolumeGLProcessor("volumemerger.frag"), vols_{{{"volume2"}, {"volume3"}, {"volume4"}}} {

    for (auto& vol : vols_) {
        addPort(vol);
        vol.setOptional(true);
    }
    addProperties(calculateDataRange_, dataRange_);
}

void VolumeMerger::preProcess(TextureUnitContainer& cont, Shader& shader, VolumeConfig& config) {
    bool needsRebuild = false;
    size_t numVolumes = 1;
    for (auto&& [i, act, vol] : std::views::zip(std::views::iota(2), active_, vols_)) {
        if (act != vol.isReady()) {
            act = vol.isReady();
            shader.getFragmentShaderObject()->setShaderDefine(fmt::format("HAS_VOL{}", i), act);
            needsRebuild = true;
        }
        if (act) ++numVolumes;
    }
    if (needsRebuild) {
        shader.build();
    }

    config.format = DataFormatBase::get(config.format->getNumericType(), numVolumes,
                                        config.format->getPrecision());

    for (auto&& [i, vol] : std::views::zip(std::views::iota(2), vols_)) {
        if (vol.isReady()) {
            utilgl::bindAndSetUniforms(shader, cont, *vol.getData(), fmt::format("vol{}", i));
        }
    }
}

}  // namespace inviwo
