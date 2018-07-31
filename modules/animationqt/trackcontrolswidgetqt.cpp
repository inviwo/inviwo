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
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QLabel>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

namespace animation {

TrackControlsWidgetQt::TrackControlsWidgetQt(QStandardItem* item, Track& track,
                                             AnimationController& controller)
    : QWidget(), controller_(controller), track_{track}, item_(item) {

    setObjectName("TrackControlsWidget");

    track_.addObserver(this);

    layout_ = new QHBoxLayout();

    layout_->setSpacing(1);
    layout_->setMargin(0);

    QSize iconSize = QSize(8, 8);

    {
        QIcon enableTrackIcon;
        enableTrackIcon.addFile(":/animation/icons/crossedeye_32.png", iconSize, QIcon::Normal,
                                QIcon::On);
        enableTrackIcon.addFile(":/animation/icons/eye_look_search_view_icon_32.png", iconSize,
                                QIcon::Normal, QIcon::Off);
        QAction* disable = new QAction(enableTrackIcon, "Enable/Disable Track");
        connect(disable, &QAction::triggered, this,
                [this]() { track_.setEnabled(!track_.isEnabled()); });

        disable->setCheckable(true);
        disable->setChecked(!track_.isEnabled());
        btnDisable_ = new QToolButton(this);
        btnDisable_->setDefaultAction(disable);
        layout_->addWidget(btnDisable_, Qt::AlignLeft | Qt::AlignVCenter);
    }

    {
        QIcon lockTrackIcon;
        lockTrackIcon.addFile(
            ":/animation/icons/account_lock_password_protect_save_saving_security_icon_32.png",
            iconSize, QIcon::Normal, QIcon::Off);
        lockTrackIcon.addFile(
            ":/animation/icons/lock_open_opened_protection_safety_security_unlocked_icon_32.png",
            iconSize, QIcon::Normal, QIcon::On);
        QAction* lock = new QAction(lockTrackIcon, "Lock/Unlock Track");
        connect(lock, &QAction::triggered, this, [this]() {
            // lock the track
            LogWarn("Locking tracks is not implemented yet.");
        });
        lock->setCheckable(true);
        lock->setChecked(false);
        btnLock_ = new QToolButton(this);
        btnLock_->setDefaultAction(lock);
        layout_->addWidget(btnLock_, Qt::AlignLeft | Qt::AlignVCenter);
    }

    {
        auto label = new QLabel(utilqt::toQString(track_.getName()));
        layout_->addWidget(label, Qt::AlignRight | Qt::AlignVCenter);
    }

    {
        QIcon prevIcon;
        prevIcon.addFile(":/animation/icons/arrow_direction_left_next_previous_return_icon_32.png",
                         iconSize, QIcon::Normal, QIcon::On);
        QAction* prev = new QAction(prevIcon, "Prev Keyframe");
        connect(prev, &QAction::triggered, this, [this]() {
            auto times = track_.getAllTimes();
            auto it = std::lower_bound(times.begin(), times.end(), controller_.getCurrentTime());
            if (it != times.begin()) {
                controller_.eval(controller_.getCurrentTime(), *std::prev(it));
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

        connect(addAndDelete, &QAction::triggered, this, [this]() {
            track_.add(controller_.getCurrentTime(), QApplication::keyboardModifiers() == Qt::CTRL);
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
        connect(next, &QAction::triggered, this, [this]() {
            auto times = track_.getAllTimes();
            auto it = std::upper_bound(times.begin(), times.end(), controller_.getCurrentTime());
            if (it != times.end()) {
                controller_.eval(controller_.getCurrentTime(), *it);
            }
        });
        auto btnNext = new QToolButton(this);
        btnNext->setDefaultAction(next);
        layout_->addWidget(btnNext, Qt::AlignRight | Qt::AlignVCenter);
    }

    setLayout(layout_);
}

void TrackControlsWidgetQt::onEnabledChanged(Track*) {
    btnDisable_->setChecked(!track_.isEnabled());
}

void TrackControlsWidgetQt::mousePressEvent(QMouseEvent*) {
    if (auto propertytrack = dynamic_cast<BasePropertyTrack*>(&track_)) {
        // Deselect all processors first
        util::setSelected(util::getInviwoApplication()->getProcessorNetwork()->getProcessors(),
                          false);
        auto property = propertytrack->getProperty();
        // Select the processor the selected property belongs to
        Processor* processor = property->getOwner()->getProcessor();
        util::setSelected({processor}, true);
    }
}

}  // namespace animation

}  // namespace inviwo
