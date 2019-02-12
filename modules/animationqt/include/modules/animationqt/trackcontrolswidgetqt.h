/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_TRACKCONTROLSWIDGETQT_H
#define IVW_TRACKCONTROLSWIDGETQT_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <modules/animation/animationcontroller.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/trackobserver.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframeobserver.h>
#include <QWidget>
#include <QStandardItem>

class QHBoxLayout;
class QToolButton;

namespace inviwo {

namespace animation {

class IVW_MODULE_ANIMATIONQT_API TrackControlsWidgetQt : public QWidget, public TrackObserver {

public:
    TrackControlsWidgetQt(QStandardItem* item, Track& track, AnimationController& controller);
    virtual ~TrackControlsWidgetQt() = default;

    Track& track();
    const Track& track() const;

private:
    // TrackObserver overloads;
    virtual void onEnabledChanged(Track* track) override;

    AnimationController& controller_;
    Track& track_;

    QHBoxLayout* layout_{nullptr};

    QToolButton* btnDisable_;
    QToolButton* btnLock_;
    QToolButton* btnAddAndDelete_;
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_TRACKCONTROLSWIDGETQT_H
