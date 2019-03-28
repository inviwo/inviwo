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

#include <modules/userinterfacegl/glui/layout/boxlayout.h>
#include <modules/userinterfacegl/glui/element.h>

#include <algorithm>

namespace inviwo {

namespace glui {

BoxLayout::BoxLayout(LayoutDirection direction) : direction_(direction), spacing_(5) {}

void BoxLayout::setDirection(LayoutDirection direction) { direction_ = direction; }

BoxLayout::LayoutDirection BoxLayout::getDirection() const { return direction_; }

ivec2 BoxLayout::getExtent() const {
    ivec2 extent(0, 0);
    if (direction_ == LayoutDirection::Vertical) {
        for (Element &elem : uiElements_) {
            if (elem.isVisible()) {
                extent.x = std::max(extent.x, elem.getExtent().x);
                extent.y += elem.getExtent().y + spacing_;
            }
        }
        if (!uiElements_.empty()) {
            // no spacing after the last element
            extent.y -= spacing_;
        }
    } else {
        // horizontal layout
        for (Element &elem : uiElements_) {
            if (elem.isVisible()) {
                extent.x += elem.getExtent().x + spacing_;
                extent.y = std::max(extent.y, elem.getExtent().y);
            }
        }
        if (!uiElements_.empty()) {
            // no spacing after the last element
            extent.x -= spacing_;
        }
    }

    // consider margins (top, left, bottom, right)
    extent.x += margins_.y + margins_.w;
    extent.y += margins_.x + margins_.z;
    return extent;
}

void BoxLayout::setSpacing(int spacing) { spacing_ = spacing; }

int BoxLayout::getSpacing() const { return spacing_; }

void BoxLayout::setScalingFactor(double factor) {
    for (Element &elem : uiElements_) {
        elem.setScalingFactor(factor);
    }
}

void BoxLayout::render(const ivec2 &topLeft, const size2_t &canvasDim) {
    ivec2 pos(topLeft + ivec2(margins_.y, -margins_.x));

    if (direction_ == LayoutDirection::Vertical) {
        // vertical layout
        for (Element &elem : uiElements_) {
            if (elem.isVisible()) {
                // consider vertical extent of UI element (lower left corner)
                pos.y -= elem.getExtent().y;
                elem.render(pos, canvasDim);
                pos.y -= spacing_;
            }
        }
    } else {
        // horizontal layout, elements are vertically centered
        // use extent of the layout - margins for layouting
        const auto extent(getExtent() - ivec2(margins_.y + margins_.w, margins_.x + margins_.z));
        // reference position is in the lower left corner
        pos.y -= extent.y;

        for (Element &elem : uiElements_) {
            if (elem.isVisible()) {
                // compute vertical offset
                ivec2 offset(0);
                offset.y = (extent.y - elem.getExtent().y) / 2;

                elem.render(pos + offset, canvasDim);
                pos.x += elem.getExtent().x + spacing_;
            }
        }
    }
}

void BoxLayout::addElement(Element &element) { uiElements_.push_back(std::ref(element)); }

void BoxLayout::insertElement(int index, Element &element) {
    if ((index < 0) || (index >= static_cast<int>(uiElements_.size()))) {
        addElement(element);
    } else {
        uiElements_.insert(uiElements_.begin() + index, std::ref(element));
    }
}

void BoxLayout::removeElement(Element &element) {
    auto it = uiElements_.begin();
    while (it != uiElements_.end()) {
        if (&(*it).get() == &element) {
            it = uiElements_.erase(it);
        } else {
            ++it;
        }
    }
}

}  // namespace glui

}  // namespace inviwo
