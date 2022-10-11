/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <inviwo/core/util/factory.h>                                   // for StandardFactory
#include <modules/animationqt/factories/sequenceeditorfactoryobject.h>  // IWYU pragma: keep
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>    // IWYU pragma: keep

#include <functional>   // for less
#include <map>          // for map
#include <memory>       // for unique_ptr
#include <string>       // for string, operator<
#include <string_view>  // for string_view
#include <vector>       // for vector

namespace inviwo {

namespace animation {

class AnimationManager;
class KeyframeSequence;
class Track;

class IVW_MODULE_ANIMATIONQT_API SequenceEditorFactory
    : public StandardFactory<SequenceEditorWidget, SequenceEditorFactoryObject, std::string_view,
                             KeyframeSequence&, Track&, AnimationManager&> {
public:
    using StandardFactory<SequenceEditorWidget, SequenceEditorFactoryObject, std::string_view,
                          KeyframeSequence&, Track&, AnimationManager&>::create;

    void registerTrackToSequenceEditorMap(std::string_view trackId, std::string_view widgetId);

    std::string getSequenceEditorId(std::string_view trackId) const;

private:
    std::map<std::string, std::string, std::less<>> trackToEditor_;
};

}  // namespace animation

}  // namespace inviwo
