/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/animationqt/animationqtsupplier.h>

#include <inviwo/core/util/exception.h>                                 // for Exception
#include <inviwo/core/util/moduleutils.h>                               // for getModuleByType
#include <modules/animationqt/animationqtmodule.h>                      // for AnimationQtModule
#include <modules/animationqt/factories/sequenceeditorfactory.h>        // for SequenceEditorFac...
#include <modules/animationqt/factories/sequenceeditorfactoryobject.h>  // for SequenceEditorFac...
#include <modules/animationqt/factories/trackwidgetqtfactory.h>         // for TrackWidgetQtFactory
#include <modules/animationqt/factories/trackwidgetqtfactoryobject.h>   // for TrackWidgetQtFact...

namespace inviwo {

namespace animation {

AnimationQtSupplier::AnimationQtSupplier(AnimationQtModule& animationQtModule)
    : animationQtModule_{animationQtModule} {}

AnimationQtSupplier::AnimationQtSupplier(InviwoApplication* app)
    : animationQtModule_{[&]() -> AnimationQtModule& {
        if (auto* animationModule = util::getModuleByType<AnimationQtModule>(app)) {
            return *animationModule;
        }
        throw Exception("Was not able to find the animation qt module");
    }()} {}

AnimationQtSupplier::~AnimationQtSupplier() { unRegisterAll(); }

TrackWidgetQtFactory& AnimationQtSupplier::getTrackWidgetQtFactory() {
    return animationQtModule_.getTrackWidgetQtFactory();
}
SequenceEditorFactory& AnimationQtSupplier::getSequenceEditorFactory() {
    return animationQtModule_.getSequenceEditorFactory();
}

void AnimationQtSupplier::registerTrackToWidgetMap(std::string_view trackId,
                                                   std::string_view widgetId) {
    getTrackWidgetQtFactory().registerTrackToWidgetMap(trackId, widgetId);
}

void AnimationQtSupplier::registerTrackToSequenceEditorMap(std::string_view trackId,
                                                           std::string_view editorId) {
    getSequenceEditorFactory().registerTrackToSequenceEditorMap(trackId, editorId);
}

void AnimationQtSupplier::unRegisterAll() {
    for (auto& elem : trackWidgetQts_) {
        animationQtModule_.getTrackWidgetQtFactory().unRegisterObject(elem.get());
    }
    trackWidgetQts_.clear();

    for (auto& elem : sequenceEditors_) {
        animationQtModule_.getSequenceEditorFactory().unRegisterObject(elem.get());
    }
    sequenceEditors_.clear();
}

}  // namespace animation

}  // namespace inviwo
