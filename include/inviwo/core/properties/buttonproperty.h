/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2021 Inviwo Foundation
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

#include <inviwo/core/properties/property.h>
#include <functional>

namespace inviwo {
/**
 * \ingroup properties
 * \brief The Button property class provides buttons that you can bind functions to.
 *
 * The button property has a widget witch creates a button and register a function to it.
 * You can only assign one function to the property.
 * To bind a function to a button property use onChange.
 * Example usage:
 *
 *     myButton_.onChange([&](){doSomthing();});
 *
 * @see ButtonPropertyWidgetQt
 */
class IVW_CORE_API ButtonProperty : public Property {

public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ButtonProperty(std::string_view identifier, std::string_view displayName, Document help,
                   std::function<void()> callback,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ButtonProperty(std::string_view identifier, std::string_view displayName,
                   std::function<void()> callback,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ButtonProperty(std::string_view identifier, std::string_view displayName, Document help,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ButtonProperty(std::string_view identifier, std::string_view displayName,
                   InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                   PropertySemantics semantics = PropertySemantics::Default);

    ButtonProperty(const ButtonProperty& rhs);
    ButtonProperty(const ButtonProperty& rhs, std::function<void()> callback);
    virtual ButtonProperty* clone() const override;
    virtual ~ButtonProperty();

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
     * Causes onChange to be called.
     * @see propertyModified
     * @see onChange
     */
    virtual void pressButton();

    virtual ButtonProperty& propertyModified() override;  // override for custom onChange behavior

    // Override Property::resetToDefaultState, to avoid calling propertyModified  on reset.
    virtual ButtonProperty& resetToDefaultState() override;

    virtual bool isDefaultState() const override;

private:
    bool buttonPressed_ = false;
};

}  // namespace inviwo
