/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/animationqt/trackcontrolswidgetqt.h>
#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QLabel>

namespace inviwo {

namespace animation {

TrackControlsWidgetQt::TrackControlsWidgetQt(QStandardItem* item, AnimationController& controller)
    : QWidget(), controller_(controller), item_(item) {

    setObjectName("TrackControlsWidget");

    layout_ = new QHBoxLayout();

    layout_->setSpacing(1);
    layout_->setMargin(0);

    QSize iconSize = QSize(8, 8);

    {
        QIcon enableTrackIcon;
        enableTrackIcon.addFile(":/animation/icons/crossedeye_32.png", iconSize, QIcon::Normal,
                                QIcon::Off);
        enableTrackIcon.addFile(":/animation/icons/eye_look_search_view_icon_32.png", iconSize,
                                QIcon::Normal, QIcon::On);
        QAction* disable = new QAction(enableTrackIcon, "Enable/Disable Track");
        connect(disable, &QAction::triggered, [&]() {
            Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
            if (track) {
                track->setEnabled(!track->isEnabled());
                btnDisable_->setChecked(!btnDisable_->isChecked());
            }
        });
        btnDisable_ = new QToolButton(this);
        btnDisable_->setDefaultAction(disable);
        btnDisable_->setCheckable(true);
        btnDisable_->setChecked(true);
        layout_->addWidget(btnDisable_, Qt::AlignLeft | Qt::AlignVCenter);
    }

    {
        QIcon lockTrackIcon;
        lockTrackIcon.addFile(
            ":/animation/icons/account_lock_password_protect_save_saving_security_icon_32.png",
            iconSize, QIcon::Normal, QIcon::On);
        lockTrackIcon.addFile(
            ":/animation/icons/lock_open_opened_protection_safety_security_unlocked_icon_32.png",
            iconSize, QIcon::Normal, QIcon::Off);
        QAction* lock = new QAction(lockTrackIcon, "Lock/Unlock Track");
        connect(lock, &QAction::triggered, [&]() {
            Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
            if (track) {
                // lock the track
                LogWarn("Locking tracks is not implemented yet.");
                btnLock_->setChecked(!btnLock_->isChecked());
            }
        });
        btnLock_ = new QToolButton(this);
        btnLock_->setDefaultAction(lock);
        btnLock_->setCheckable(true);
        btnLock_->setChecked(false);
        layout_->addWidget(btnLock_, Qt::AlignLeft | Qt::AlignVCenter);
    }

    Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
    if (track) {
        auto label = new QLabel(track->getName().c_str());
        layout_->addWidget(label, Qt::AlignRight | Qt::AlignVCenter);
    }

    {
        QIcon prevIcon;
        prevIcon.addFile(":/animation/icons/arrow_direction_left_next_previous_return_icon_32.png",
                         iconSize, QIcon::Normal, QIcon::On);
        QAction* prev = new QAction(prevIcon, "Prev Keyframe");
        connect(prev, &QAction::triggered, [&]() {
            Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
            if (track) {
                auto times = track->getAllTimes();
                auto it =
                    std::lower_bound(times.begin(), times.end(), controller_.getCurrentTime());
                if (it != times.begin()) {
                    controller_.eval(controller_.getCurrentTime(), *std::prev(it));
                }
            }
        });
        QToolButton* btnprev = new QToolButton(this);
        btnprev->setDefaultAction(prev);
        layout_->addWidget(btnprev, Qt::AlignRight | Qt::AlignVCenter);
    }

    {
        QIcon keyFrameHandlingIcon;
        keyFrameHandlingIcon.addFile(":/animation/icons/add_create_new_plus_icon_32.png", iconSize,
                                     QIcon::Normal, QIcon::On);
        keyFrameHandlingIcon.addFile(
            ":/animation/icons/basket_delete_garbage_trash_waste_icon_32.png", iconSize,
            QIcon::Normal, QIcon::Off);
        QAction* addAndDelete = new QAction(keyFrameHandlingIcon, "Add Keyframe");
        connect(addAndDelete, &QAction::triggered, [&]() {
            Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
            BasePropertyTrack* propertytrack = dynamic_cast<BasePropertyTrack*>(track);
            // Might not have been a BasePropertyTrack
            if (propertytrack && btnAddAndDelete_->isChecked()) {
                auto property = propertytrack->getProperty();
                auto app = controller_.getInviwoApplication();
                auto& am = app->template getModuleByType<AnimationModule>()->getAnimationManager();
                am.addKeyframeCallback(property);
            }
        });
        btnAddAndDelete_ = new QToolButton(this);
        btnAddAndDelete_->setDefaultAction(addAndDelete);
        btnAddAndDelete_->setCheckable(true);
        btnAddAndDelete_->setChecked(true);
        layout_->addWidget(btnAddAndDelete_, Qt::AlignRight | Qt::AlignVCenter);
    }

    {
        QIcon nextIcon;
        nextIcon.addFile(":/animation/icons/arrow_direction_previous_right_icon_32.png", iconSize,
                         QIcon::Normal, QIcon::On);
        QAction* next = new QAction(nextIcon, "Next Keyframe");
        connect(next, &QAction::triggered, [&]() {
            Track* track = reinterpret_cast<Track*>(item_->data((Qt::UserRole + 1)).value<void*>());
            if (track) {
                auto times = track->getAllTimes();
                auto it =
                    std::upper_bound(times.begin(), times.end(), controller_.getCurrentTime());
                if (it != times.end()) {
                    controller_.eval(controller_.getCurrentTime(), *it);
                }
            }
        });
        QToolButton* btnNext = new QToolButton(this);
        btnNext->setDefaultAction(next);
        layout_->addWidget(btnNext, Qt::AlignRight | Qt::AlignVCenter);
    }

    setLayout(layout_);
}

}  // namespace animation

}  // namespace inviwo
