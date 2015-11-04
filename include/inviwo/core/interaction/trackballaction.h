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

#ifndef IVW_TRACKBALLACTION_H
#define IVW_TRACKBALLACTION_H

#include <inviwo/core/interaction/action.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/callback.h>

namespace inviwo {

class IVW_CORE_API TrackballAction : public Action {
public:
    enum Actions {
        TRACKBALL_ROTATE  =      0,
        TRACKBALL_ZOOM    ,
        TRACKBALL_PAN     ,
        TRACKBALL_STEPROTATE_UP,
        TRACKBALL_STEPROTATE_LEFT,
        TRACKBALL_STEPROTATE_DOWN,
        TRACKBALL_STEPROTATE_RIGHT,
        TRACKBALL_STEPZOOM_IN,
        TRACKBALL_STEPZOOM_OUT,
        TRACKBALL_STEPPAN_UP,
        TRACKBALL_STEPPAN_LEFT,
        TRACKBALL_STEPPAN_DOWN,
        TRACKBALL_STEPPAN_RIGHT,
        COUNT
    };

    template <typename T>
    TrackballAction(TrackballAction::Actions action, T* obj, void (T::*m)(Event*));

    TrackballAction(TrackballAction::Actions action);
    ~TrackballAction();

    virtual std::string getClassIdentifier() const { return "TrackballAction"; }

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

private:
    static const std::string actionNames_[COUNT];
    int action_;
};

template <typename T>
inviwo::TrackballAction::TrackballAction(TrackballAction::Actions action, T* obj,
                                         void (T::*m)(Event*))
    : Action(actionNames_[action], obj, m) 
    , action_(action) {
}

} // namespace

#endif // IVW_TRACKBALLACTION_H