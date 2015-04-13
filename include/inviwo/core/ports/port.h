/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_PORT_H
#define IVW_PORT_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/propertyowner.h>

namespace inviwo {

class Processor;
class MultiInport;

/**
 * \class Port
 *
 * \brief A port can be connected to another port and is owned by a processor.
 */
class IVW_CORE_API Port : public IvwSerializable {

    friend class Processor;
    friend class MultiInport;

public:
    /**
     * Constructor for creating port instances. As this class is abstract, the constructor is not
     * called directly. Instead, constructors of the derived classes call this constructor.
     *
     * @param identifier Port identifier used for serialization. Should be unique within the scope
     *                   of a processor.
     * @see Processor::addPort()
     */
    Port(std::string identifier = "");
    virtual ~Port();

    /**
     * Returns the RGB color code used to colorize all ports of this type. This color code is for
     * instance used in the NetworkEditor. To distinguish different port types through their color,
     * this method should be overloaded in derived classes.
     */
    virtual uvec3 getColorCode() const;
    /**
     * All instances have the same color.
     * Derived should declare its own and return DerivedClass::colorCode in getColorCode
     */
    static uvec3 colorCode;

    Processor* getProcessor() const;
    std::string getIdentifier() const;

    virtual std::string getClassIdentifier() const = 0;
    virtual std::string getContentInfo() const = 0;

    virtual bool isConnected() const = 0;
    virtual bool isReady() const = 0;

    virtual void invalidate(InvalidationLevel invalidationLevel);
    virtual InvalidationLevel getInvalidationLevel() const { return INVALID_OUTPUT; }
    virtual void setInvalidationLevel(InvalidationLevel invalidationLevel) = 0;

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

protected:
    std::string identifier_;

    void setIdentifier(const std::string& name);
    void setProcessor(Processor* processor);

private:
    Processor* processor_;
};

} // namespace

#endif // IVW_PORT_H
