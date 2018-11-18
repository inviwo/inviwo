/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2018 Inviwo Foundation
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

#ifndef IVW_GLUIBUTTONPROPERTYWIDGET_H
#define IVW_GLUIBUTTONPROPERTYWIDGET_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/widgets/button.h>
#include <modules/userinterfacegl/glui/widgets/toolbutton.h>

#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertyobserver.h>

namespace inviwo {

class Texture2D;

namespace glui {

/**
 * \class ButtonProperty
 * \brief GLUI property widget for a button property using a glui Button
 */
class IVW_MODULE_USERINTERFACEGL_API ButtonPropertyWidget : public Button,
                                                            public PropertyWidget,
                                                            public PropertyObserver {
public:
    ButtonPropertyWidget(ButtonProperty &property, Processor &processor, Renderer &uiRenderer,
                         const ivec2 &extent = ivec2(100, 24));
    virtual ~ButtonPropertyWidget() = default;

    virtual void updateFromProperty() override;

    // PropertyObservable overrides
    virtual void onSetVisible(Property *property, bool visible) override;
    virtual void onSetDisplayName(Property *property, const std::string &displayName) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;

private:
    ButtonProperty *property_;
};

/**
 * \class ToolButtonProperty
 * \brief GLUI property widget for a button property using a glui ToolButton
 */
class IVW_MODULE_USERINTERFACEGL_API ToolButtonPropertyWidget : public ToolButton,
                                                                public PropertyWidget,
                                                                public PropertyObserver {
public:
    ToolButtonPropertyWidget(const std::string &imageFileName, ButtonProperty &property,
                             Processor &processor, Renderer &uiRenderer,
                             const ivec2 &extent = ivec2(24, 24));
    ToolButtonPropertyWidget(ButtonProperty &property, std::shared_ptr<Texture2D> image,
                             Processor &processor, Renderer &uiRenderer,
                             const ivec2 &extent = ivec2(24, 24));
    virtual ~ToolButtonPropertyWidget() = default;

    virtual void updateFromProperty() override;

    // PropertyObservable overrides
    virtual void onSetVisible(Property *property, bool visible) override;
    virtual void onSetDisplayName(Property *property, const std::string &displayName) override;
    virtual void onSetReadOnly(Property* property, bool readonly) override;

private:
    ButtonProperty *property_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUIBUTTONPROPERTYWIDGET_H
