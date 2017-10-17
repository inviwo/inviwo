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

#ifndef IVW_GLUIMANAGER_H
#define IVW_GLUIMANAGER_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <modules/userinterfacegl/glui/element.h>
#include <modules/userinterfacegl/glui/renderer.h>
#include <modules/userinterfacegl/glui/layout/layout.h>

#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/ports/imageport.h>

#include <vector>

namespace inviwo {

class Processor;
class PickingObject;

namespace glui {

/**
 * \class glui::Manager
 * \brief provides a simple UI manager based on OpenGL. This class is responsible for 
 * managing glui::Elements. It provides a intuitive interface for rendering single UI 
 * elements and layouts.
 * 
 * \see glui::Element
 */
class IVW_MODULE_USERINTERFACEGL_API Manager { 
public:
    Manager(Processor *processor);
    virtual ~Manager() = default;
    
    void setTextColor(const vec4 &color);

    void setUIColor(const vec4 &color);

    void setHoverColor(const vec4 &color);

    void renderLayout(Layout &layout, const ivec2 &origin, const ImageOutport &outport);

    void renderUIElement(Element &element, const ivec2 &origin, const ImageOutport &outport);

    Element* createUIElement(ItemType type, const std::string &label, const ivec2 &extent);

    /**
     * \brief add the given UI element to the manager in order to handle picking. The manager takes
     * ownership of the element.
     *
     * @param element    UI element to be managed
     */
    void addUIElement(Element *element);
    
    Renderer& getUIRenderer();
    const Renderer& getUIRenderer() const;

private:
    void handlePickingEvent(PickingEvent *e);
    void updatePickingIDs(Element *element);

    PickingMapper pickingMapper_;
    Processor *processor_;
    
    Renderer uiRenderer_;

    std::vector<std::unique_ptr<Element>> uiElements_;
};

} // namespace glui

} // namespace inviwo

#endif // IVW_GLUIMANAGER_H
