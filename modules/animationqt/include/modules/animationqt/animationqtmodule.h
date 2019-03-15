/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_ANIMATIONQTMODULE_H
#define IVW_ANIMATIONQTMODULE_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>

#include <modules/animationqt/factories/sequenceeditorfactory.h>
#include <modules/animationqt/factories/trackwidgetqtfactory.h>
#include <modules/animationqt/animationqtsupplier.h>

class QMenu;

namespace inviwo {

namespace animation {
class AnimationEditorDockWidgetQt;
class DemoNavigatorDockWidgetQt;
}  // namespace animation

class IVW_MODULE_ANIMATIONQT_API AnimationQtModule : public InviwoModule,
                                                     public animation::AnimationQtSupplier {
public:
    AnimationQtModule(InviwoApplication* app);
    virtual ~AnimationQtModule();

    animation::TrackWidgetQtFactory& getTrackWidgetQtFactory();
    const animation::TrackWidgetQtFactory& getTrackWidgetQtFactory() const;

    animation::SequenceEditorFactory& getSequenceEditorFactory();
    const animation::SequenceEditorFactory& getSequenceEditorFactory() const;

private:
    animation::TrackWidgetQtFactory trackWidgetQtFactory_;
    animation::SequenceEditorFactory sequenceEditorFactory_;

    // Keep references to added widgets so that they can be removed in destructor
    std::unique_ptr<animation::AnimationEditorDockWidgetQt> editor_;
    std::unique_ptr<animation::DemoNavigatorDockWidgetQt> navigator_;

    std::unique_ptr<QMenu> menu_;  // Show/hide animation editor
};

}  // namespace inviwo

#endif  // IVW_ANIMATIONQTMODULE_H
