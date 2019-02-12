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

#ifndef IVW_SEQUENCEEDITORFACTORYOBJECT_H
#define IVW_SEQUENCEEDITORFACTORYOBJECT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/animationmanager.h>
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATIONQT_API SequenceEditorFactoryObject {
public:
    SequenceEditorFactoryObject(const std::string& classIdentifier);
    virtual ~SequenceEditorFactoryObject() = default;

    virtual std::unique_ptr<SequenceEditorWidget> create(KeyframeSequence&, Track&,
                                                         AnimationManager&) const = 0;
    const std::string& getClassIdentifier() const;

protected:
    const std::string classIdentifier_;
};

template <typename T>
class SequenceEditorFactoryObjectTemplate : public SequenceEditorFactoryObject {
public:
    // Requires a static classIdentifier() method on T
    SequenceEditorFactoryObjectTemplate() : SequenceEditorFactoryObject(T::classIdentifier()) {}

    SequenceEditorFactoryObjectTemplate(const std::string& classIdentifier)
        : SequenceEditorFactoryObject(classIdentifier){};
    virtual ~SequenceEditorFactoryObjectTemplate() = default;

    virtual std::unique_ptr<SequenceEditorWidget> create(KeyframeSequence& sequence, Track& track,
                                                         AnimationManager& manager) const override {
        return std::make_unique<T>(sequence, track, manager);
    }
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_SEQUENCEEDITORFACTORYOBJECT_H
