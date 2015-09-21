/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_BUTTONPROPERTY_H
#define IVW_BUTTONPROPERTY_H

#include <inviwo/core/util/callback.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {
/** class ButtonProperty
 * \brief The Button property class provides buttons that you can bind functions to.
 *
 * The button property has a widget witch creates a button and register a function to it.
 * You can only assign one function to the property.
 * To bind a function to a button property use onChange.
 * Example usage
 * myButton_.onChange(this, &MyButton::doSomethingFunction);
 * A button property are normally used by a processor.
 * The button property is created and added in the constructor of the processor.
 *
 * @see ButtonPropertyWidgetQt
 */
class IVW_CORE_API ButtonProperty : public Property {

public:
    InviwoPropertyInfo();
    ButtonProperty(std::string identifier,
                   std::string displayName,
                   InvalidationLevel invalidationLevel=INVALID_OUTPUT,
                   PropertySemantics semantics = PropertySemantics::Default);
    

    ButtonProperty(const ButtonProperty& rhs);
    ButtonProperty& operator=(const ButtonProperty& that);
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

    // Override Property::resetToDefaultState, to avoid calling propertyModified  on reset.
    virtual void resetToDefaultState() override;
};

} //namespace

#endif //IVW_BUTTONPROPERTY_H