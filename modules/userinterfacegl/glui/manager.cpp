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

#include <modules/userinterfacegl/glui/manager.h>

#include <modules/userinterfacegl/glui/element.h>
#include <modules/userinterfacegl/glui/widgets/button.h>
#include <modules/userinterfacegl/glui/widgets/checkbox.h>
#include <modules/userinterfacegl/glui/widgets/slider.h>

#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/pickingmanager.h>

#include <numeric>
#include <algorithm>

namespace inviwo {

namespace glui {

Manager::Manager(Processor *processor)
    : pickingMapper_(processor, 1, [&](PickingEvent *e) { handlePickingEvent(e); })
    , processor_(processor) {}

void Manager::setTextColor(const vec4 &color) { uiRenderer_.setTextColor(color); }

void Manager::setUIColor(const vec4 &color) { uiRenderer_.setUIColor(color); }

void Manager::setHoverColor(const vec4 &color) { uiRenderer_.setHoverColor(color); }

void Manager::renderLayout(Layout &layout, const ivec2 &origin, const ImageOutport &outport) {
    layout.render(origin, pickingMapper_, outport.getDimensions());
}

void Manager::renderUIElement(Element &element, const ivec2 &origin, const ImageOutport &outport) {
    element.render(origin, pickingMapper_, outport.getDimensions());

    // render text label
    element.renderLabel(origin, outport.getDimensions());
}

void Manager::addUIElement(Element *element) {
    updatePickingIDs(element);

    uiElements_.emplace_back(element);
}

Element *Manager::createUIElement(ItemType type, const std::string &label, const ivec2 &extent) {
    std::unique_ptr<Element> element = nullptr;
    switch (type) {
        case ItemType::Button:
            element = util::make_unique<Button>(&uiRenderer_, label, extent);
            break;
        case ItemType::Checkbox:
            element = util::make_unique<CheckBox>(&uiRenderer_, label, extent);
            break;
        case ItemType::Slider:
            element = util::make_unique<Slider>(&uiRenderer_, label, 50, 0, 100, extent);
            break;
        case ItemType::Unknown:
        default:
            LogError("GLUIManager: unsupported item type.");
            break;
    }

    if (element) {
        updatePickingIDs(element.get());

        uiElements_.emplace_back(std::move(element));
        return uiElements_.back().get();
    } else {
        return nullptr;
    }
}

const Renderer &Manager::getUIRenderer() const { return uiRenderer_; }

Renderer &Manager::getUIRenderer() { return uiRenderer_; }

void Manager::handlePickingEvent(PickingEvent *e) {
    if (e->getEvent()->hash() == MouseEvent::chash()) {
        auto mouseEvent = e->getEventAs<MouseEvent>();

        bool leftMouseBtn = (mouseEvent->button() == MouseButton::Left);
        bool mousePress = (mouseEvent->state() == MouseState::Press);
        bool mouseRelease = (mouseEvent->state() == MouseState::Release);
        bool mouseMove = (mouseEvent->state() == MouseState::Move);

        const auto pickedID = static_cast<int>(e->getPickedId());

        // find UI element matching the picked ID
        auto it = std::find_if(uiElements_.begin(), uiElements_.end(),
                               [pickedID](auto &elem) { return elem->hasPickingID(pickedID); });

        if (it != uiElements_.end()) {
            bool triggerUpdate = false;
            if (e->getState() == PickingState::Started) {
                (*it)->setHoverState(true);
                triggerUpdate = true;
            } else if (e->getState() == PickingState::Finished) {
                (*it)->setHoverState(false);
                triggerUpdate = true;
            } else if (e->getState() == PickingState::Updated) {
                if (mouseMove && (mouseEvent->buttonState() & MouseButton::Left) == MouseButton::Left) {
                    auto delta = e->getDeltaPressedPosition();
                    triggerUpdate = (*it)->moveAction(delta * dvec2(e->getCanvasSize()));

                    e->markAsUsed();
                }
                else if (leftMouseBtn) {
                    if (mousePress) {
                        // initial activation with mouse button press
                        (*it)->setPushedState(true);
                        triggerUpdate = true;
                    } else if (mouseRelease && (*it)->isPushed()) {
                        // mouse button is release upon the active element
                        (*it)->triggerAction();
                        (*it)->setPushedState(false);
                        triggerUpdate = true;
                    }
                    /* // TODO: mouse drag not yet implemented!
                    else if (mouseMove && (*it)->isPushed()) {
                        // mouse has been moved while the mouse button is stilled pressed
                        ivec2 delta(e->getDeltaPosition() * dvec2(e->getCanvasSize()));
                        triggerUpdate = (*it)->moveAction(delta);
                    }
                    */
                }
            }
            if (triggerUpdate) {
                processor_->invalidate(InvalidationLevel::InvalidOutput);
            }
        }
    }
}

void Manager::updatePickingIDs(Element *element) {
    // sum up the number of picking IDs required by all existing UI elements
    auto numPickIDs =
        std::accumulate(uiElements_.begin(), uiElements_.end(), 0,
                        [](int v, auto &elem) { return v + elem->getNumWidgetComponents(); });

    numPickIDs = element->updatePickingIDs(numPickIDs);

    if (pickingMapper_.getSize() < static_cast<std::size_t>(numPickIDs)) {
        // adjust number of utilized picking IDs in the picking mapper
        pickingMapper_.resize(numPickIDs);
    }
}

}  // namespace glui

}  // namespace inviwo
