/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <string_view>
#include <ranges>

#include <glm/vec3.hpp>

namespace inviwo {

TFSelectionWatcher::TFSelectionWatcher(Property* property, std::span<TFPrimitiveSet*> sets)
    : property_(property), tfSets_(sets.begin(), sets.end()) {}

void TFSelectionWatcher::setPosition(double pos) {
    const NetworkLock lock(property_);
    const util::KeepTrueWhileInScope b(&updateInProgress_);
    for (auto& elem : tfSets_) {
        elem->setPosition(selectedPrimitives_, pos);
    }
    emit updateWidgetPosition(pos) ;
}

void TFSelectionWatcher::setAlpha(double alpha) {
    const NetworkLock lock(property_);
    const util::KeepTrueWhileInScope b(&updateInProgress_);
    std::ranges::for_each(selectedPrimitives_,
                          [&](TFPrimitive* p) { p->setAlpha(static_cast<float>(alpha)); });
    emit updateWidgetAlpha(alpha);
}

void TFSelectionWatcher::setColor(const QColor& c) {
    const NetworkLock lock(property_);
    const util::KeepTrueWhileInScope b(&updateInProgress_);
    const auto color = utilqt::tovec3(c);
    std::ranges::for_each(selectedPrimitives_, [&](TFPrimitive* p) { p->setColor(color); });

    emit updateWidgetColor(c);
}

void TFSelectionWatcher::updateSelection(const std::vector<TFPrimitive*>& selection) {
    // de-register all primitive callbacks
    for (auto* p : selectedPrimitives_) {
        p->removeObserver(this);
    }

    // observe all primitives in new selection
    selectedPrimitives_.clear();
    for (auto* p : selection) {
        selectedPrimitives_.push_back(p);
        p->addObserver(this);
    }

    informWidgets();
}

void TFSelectionWatcher::onTFPrimitiveChange(const TFPrimitive& p) {
    if (updateInProgress_) return;

    if (selectedPrimitives_.empty() || !util::contains(selectedPrimitives_, &p)) {
        throw Exception("TF primitive callback detected on non-selected primitive");
    }

    informWidgets();
}

void TFSelectionWatcher::informWidgets() {
    // check whether the position/alpha/color of all selected primitives is identical
    if (selectedPrimitives_.empty()) {
        // nothing selected, mark as ambiguous and use pos = 0, alpha = 0, color = [0,0,0]
        emit updateWidgetPosition(std::nullopt, false);
        emit updateWidgetAlpha(std::nullopt, false);
        emit updateWidgetColor(std::nullopt, false);
    } else {
        const auto* p = selectedPrimitives_.front();

        bool ambiguousPos = false;
        bool ambiguousAlpha = false;
        bool ambiguousColor = false;

        for (const auto* primitive : selectedPrimitives_ | std::views::drop(1)) {
            ambiguousPos |= (p->getPosition() != primitive->getPosition());
            ambiguousAlpha |= (p->getAlpha() != primitive->getAlpha());
            ambiguousColor |= (vec3(p->getColor()) != vec3(primitive->getColor()));
        }

        emit updateWidgetPosition(ambiguousPos ? std::nullopt : std::optional{p->getPosition()},
                                  ambiguousPos);
        emit updateWidgetAlpha(ambiguousAlpha ? std::nullopt : std::optional{p->getAlpha()},
                               ambiguousAlpha);
        emit updateWidgetColor(
            ambiguousColor ? std::nullopt : std::optional{utilqt::toQColor(p->getColor())},
            ambiguousColor);
    }
}

}  // namespace inviwo
