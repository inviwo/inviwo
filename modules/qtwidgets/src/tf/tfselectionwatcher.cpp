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

#include <modules/qtwidgets/tf/tfselectionwatcher.h>
#include <modules/qtwidgets/tf/tfeditor.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

TFSelectionWatcher::TFSelectionWatcher(TFEditor *editor, Property *property,
                                       const std::vector<TFPrimitiveSet *> &primitiveSets)
    : tfEditor_(editor), property_(property), tfSets_(primitiveSets) {}

void TFSelectionWatcher::setPosition(double pos) {
    NetworkLock lock(property_);
    util::KeepTrueWhileInScope b(&updateInProgress_);
    for (auto &elem : tfSets_) {
        elem->setPosition(selectedPrimitives_, pos);
    }
    emit updateWidgetPosition(pos, false);
}

void TFSelectionWatcher::setAlpha(double alpha) {
    NetworkLock lock(property_);
    util::KeepTrueWhileInScope b(&updateInProgress_);
    for (auto &elem : tfSets_) {
        elem->setAlpha(selectedPrimitives_, alpha);
    }
    emit updateWidgetAlpha(alpha, false);
}

void TFSelectionWatcher::setColor(const QColor &c) {
    NetworkLock lock(property_);
    util::KeepTrueWhileInScope b(&updateInProgress_);
    for (auto &elem : tfSets_) {
        elem->setColor(selectedPrimitives_, utilqt::tovec3(c));
    }
    emit updateWidgetColor(c, false);
}

void TFSelectionWatcher::updateSelection(const std::vector<TFPrimitive *> selection) {
    // de-register all primitive callbacks
    for (auto p : selectedPrimitives_) {
        p->removeObserver(this);
    }

    // observe all primitives in new selection
    selectedPrimitives_.clear();
    for (auto p : selection) {
        selectedPrimitives_.push_back(p);
        p->addObserver(this);
    }

    informWidgets();
}

void TFSelectionWatcher::onTFPrimitiveChange(const TFPrimitive &p) {
    if (updateInProgress_) return;

    if (selectedPrimitives_.empty() || !util::contains(selectedPrimitives_, &p)) {
        throw Exception("TF primitive callback detected on non-selected primitive", IVW_CONTEXT);
    }

    informWidgets();
}

void TFSelectionWatcher::informWidgets() {
    // check whether the position/alpha/color of all selected primitives is identical
    if (selectedPrimitives_.empty()) {
        // nothing selected, mark as ambiguous and use pos = 0, alpha = 0, color = [0,0,0]
        emit updateWidgetPosition(0.0, true);
        emit updateWidgetAlpha(0.0, true);
        emit updateWidgetColor(QColor(), true);
    } else {
        auto p = selectedPrimitives_.front();

        // cache primitive data if it is a single element selection
        if (selectedPrimitives_.size() == 1) {
            cachedPos_ = p->getPosition();
            cachedAlpha_ = p->getAlpha();
            cachedColor_ = utilqt::toQColor(p->getColor());
        }

        bool ambiguousPos = false;
        bool ambiguousAlpha = false;
        bool ambiguousColor = false;

        for (auto primitive : selectedPrimitives_) {
            ambiguousPos |= (p->getPosition() != primitive->getPosition());
            ambiguousAlpha |= (p->getAlpha() != primitive->getAlpha());
            ambiguousColor |= !(vec3(p->getColor()) == vec3(primitive->getColor()));
        }

        emit updateWidgetPosition(ambiguousPos ? cachedPos_ : p->getPosition(), ambiguousPos);
        emit updateWidgetAlpha(ambiguousAlpha ? cachedAlpha_ : p->getAlpha(), ambiguousAlpha);
        emit updateWidgetColor(ambiguousColor ? cachedColor_ : utilqt::toQColor(p->getColor()),
                               ambiguousColor);
    }
}

}  // namespace inviwo
