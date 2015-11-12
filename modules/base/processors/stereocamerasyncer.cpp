/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "stereocamerasyncer.h"
#include <inviwo/core/util/raiiutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo StereoCameraSyncer::processorInfo_{
    "org.inviwo.StereoCameraSyncer",  // Class identifier
    "Stereo Camera Syncer",           // Display name
    "Undefined",                      // Category
    CodeState::Experimental,          // Code state
    Tags::None,                       // Tags
};
const ProcessorInfo StereoCameraSyncer::getProcessorInfo() const { return processorInfo_; }

StereoCameraSyncer::StereoCameraSyncer()
    : Processor()
    , separation_("separation", "Separation", 0.1f, 0.f, 1.f, 0.01f, InvalidationLevel::Valid)
    , master_("master", "Master", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f),
              nullptr, InvalidationLevel::Valid)
    , left_("left", "Left", master_.getLookFrom() + separation_.get() / 2.0f, master_.getLookTo(),
            master_.getLookUp(), nullptr, InvalidationLevel::Valid)
    , right_("right", "right", master_.getLookFrom() - separation_.get() / 2.0f,
             master_.getLookTo(), master_.getLookUp(), nullptr, InvalidationLevel::Valid) {
    addProperty(separation_);
    
    addProperty(master_);
    addProperty(left_);
    addProperty(right_);

    separation_.onChange([&]() {
        if (isChanging) return;
        util::KeepTrueWhileInScope guard(&isChanging);
        pushMaster();
    });

    master_.onChange([&](){
        if(isChanging) return;
        util::KeepTrueWhileInScope guard(&isChanging);
        pushMaster();
    });

    left_.onChange([&]() {
        if (isChanging) return;
        util::KeepTrueWhileInScope guard(&isChanging);
        // TODO implement

    });

    right_.onChange([&]() {
        if (isChanging) return;
        util::KeepTrueWhileInScope guard(&isChanging);
        // TODO implement

    });

}

void StereoCameraSyncer::process() {
}

void StereoCameraSyncer::pushMaster() {
    left_.setLookTo(master_.getLookTo());
    left_.setLookUp(master_.getLookUp());
    left_.setLookFrom(master_.getLookFrom() - 0.5f * master_.getLookRight()*separation_.get());

    right_.setLookTo(master_.getLookTo());
    right_.setLookUp(master_.getLookUp());
    right_.setLookFrom(master_.getLookFrom() + 0.5f * master_.getLookRight()*separation_.get());
}

}  // namespace
