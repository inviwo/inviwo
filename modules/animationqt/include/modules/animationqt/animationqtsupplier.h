/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_ANIMATIONQTSUPPLIER_H
#define IVW_ANIMATIONQTSUPPLIER_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animationqt/factories/sequenceeditorfactory.h>
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animationqt/factories/sequenceeditorfactoryobject.h>
#include <modules/animationqt/factories/trackwidgetqtfactoryobject.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

class AnimationQtModule;

namespace animation {

class IVW_MODULE_ANIMATIONQT_API AnimationQtSupplier {
public:
    AnimationQtSupplier(AnimationQtModule& animationQtModule);
    AnimationQtSupplier(InviwoApplication* app);
    AnimationQtSupplier(const AnimationQtSupplier&) = delete;
    AnimationQtSupplier& operator=(const AnimationQtSupplier&) = delete;
    virtual ~AnimationQtSupplier();

    /**
     * Register a Track with the Track Factory
     */
    template <typename T>
    void registerTrackWidgetQt();

    void registerTrackToWidgetMap(const std::string& trackId, const std::string& widgetId);

    /**
     * Register a Track with the Track Factory
     */
    template <typename T>
    void registerSequenceEditor();

    void registerTrackToSequenceEditorMap(const std::string& trackId, const std::string& erditorId);

    void unRegisterAll();

private:
    TrackWidgetQtFactory& getTrackWidgetQtFactory();
    SequenceEditorFactory& getSequenceEditorFactory();
    AnimationQtModule& animationQtModule_;
    std::vector<std::unique_ptr<TrackWidgetQtFactoryObject>> trackWidgetQts_;
    std::vector<std::unique_ptr<SequenceEditorFactoryObject>> sequenceEditors_;
};

template <typename T>
void AnimationQtSupplier::registerTrackWidgetQt() {
    auto track = std::make_unique<TrackWidgetQtFactoryObjectTemplate<T>>();
    if (getTrackWidgetQtFactory().registerObject(track.get())) {
        trackWidgetQts_.push_back(std::move(track));
    }
}
template <typename T>
void AnimationQtSupplier::registerSequenceEditor() {
    auto track = std::make_unique<SequenceEditorFactoryObjectTemplate<T>>();
    if (getSequenceEditorFactory().registerObject(track.get())) {
        sequenceEditors_.push_back(std::move(track));
    }
}

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_ANIMATIONQTSUPPLIER_H
