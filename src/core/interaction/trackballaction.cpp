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

#include <inviwo/core/interaction/trackballaction.h>

namespace inviwo {

TrackballAction::TrackballAction(TrackballAction::Actions action)
    : Action(actionNames_[action])
    , action_(action) {
}

TrackballAction::~TrackballAction() {}

void TrackballAction::serialize(IvwSerializer& s) const {
    Action::serialize(s);
    s.serialize("action", name_);
}
void TrackballAction::deserialize(IvwDeserializer& d) {
    d.deserialize("action", name_);

    for (int i = 0; i < COUNT; ++i) {
        if (actionNames_[i] == name_) {
            action_ = i;
            break;
        }
    }
}

const std::string TrackballAction::actionNames_[COUNT] = {
    "Trackball rotate",
    "Trackball zoom",
    "Trackball pan",
    "Step rotate up",
    "Step rotate left",
    "Step rotate down",
    "Step rotate right",
    "Step zoom in",
    "Step zoom out",
    "Step pan up",
    "Step pan left",
    "Step pan down",
    "Step pan right" };

} //namespace