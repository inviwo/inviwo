/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/property.h>
#include <functional>
#include <optional>

namespace inviwo {

/**
 * \ingroup properties
 * \brief The Button Group property class provides a group of button that you can bind functions to.
 * The button group property has a widget which creates a row of buttons and register functions to
 * them.
 * @see ButtonGroupPropertyWidgetQt
 */
class IVW_CORE_API ButtonGroupProperty : public Property {
public:
    struct IVW_CORE_API Button {
        std::optional<std::string> name;
        std::optional<std::string> icon;
        std::optional<std::string> tooltip;
        std::function<void()> action;
    };

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ButtonGroupProperty(std::string identifier, std::string displayName,
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                        PropertySemantics semantics = PropertySemantics::Default);

    ButtonGroupProperty(std::string identifier, std::string displayName,
                        std::vector<Button> buttons,
                        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                        PropertySemantics semantics = PropertySemantics::Default);

    ButtonGroupProperty(const ButtonGroupProperty& rhs);
    ButtonGroupProperty(const ButtonGroupProperty& rhs, std::vector<Button> buttons);
    virtual ButtonGroupProperty* clone() const override;
    virtual ~ButtonGroupProperty() = default;

    void addButton(Button button);
    const Button& getButton(size_t i) const;
    size_t size() const;

    /**
     * Overrides the default implementation to
     * enable buttons to be linked.
     * Will cause the button to be pressed
     * when a linked button is pressed.
     *
     * @param src Button property
     */
    virtual void set(const Property* src) override;

    /**
     * Causes callback to be called.
     * @see propertyModified
     * @see onChange
     */
    void pressButton(size_t index);

    virtual ButtonGroupProperty& propertyModified()
        override;  // override for custom onChange behavior

    // Override Property::resetToDefaultState, to avoid calling propertyModified  on reset.
    virtual ButtonGroupProperty& resetToDefaultState() override;

private:
    std::vector<Button> buttons_;

    size_t buttonPressed_ = std::numeric_limits<size_t>::max();
};

}  // namespace inviwo
