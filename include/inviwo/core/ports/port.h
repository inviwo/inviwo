/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/document.h>

#include <glm/fwd.hpp>
#include <string>

namespace inviwo {

class Processor;
class Document;

/**
 * \defgroup ports Ports
 */

/**
 * \ingroup ports
 * \brief A abstract base class for all ports.
 * A port can be connected to other ports and is owned by a processor.
 */
class IVW_CORE_API Port : public Serializable {
    friend class Processor;

public:
    Port(const Port&) = delete;
    Port& operator=(const Port&) = delete;
    Port(Port&&) = delete;
    Port& operator=(Port&&) = delete;

    virtual ~Port() = default;

    const std::string& getIdentifier() const;
    void setIdentifier(const std::string& name);
    Processor* getProcessor() const;

    /**
     * @brief Get the port path i.e. ```<processor identifier>.<port identifier>```
     */
    std::string getPath() const;

    virtual std::string getClassIdentifier() const = 0;
    /**
     * Returns the RGB color code used to colorize all ports of this type. This color code is for
     * instance used in the NetworkEditor. To distinguish different port types through their color,
     * this method should be overloaded in derived classes.
     */
    virtual glm::uvec3 getColorCode() const = 0;

    /**
     * This function should describe the state of the port and the data it holds.
     * Derived ports should extend this function and add information about their state.
     * The port help is usually included in this information.
     * The description is usually shown as a tooltip in the GUI.
     */
    virtual Document getInfo() const = 0;

    virtual bool isConnected() const = 0;
    virtual bool isReady() const = 0;

    /**
     * The help should describe what data the port expects/produces and how it is used.
     * This will be shown in the Processor help, and as part of the port info in the
     * port tooltip in the GUI
     */
    const Document& getHelp() const;
    Document& getHelp();
    Port& setHelp(Document help);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

protected:
    /**
     * Constructor for creating port instances. As this class is abstract, the constructor is not
     * called directly. Instead, constructors of the derived classes call this constructor.
     * @param identifier Port identifier used for serialization. Has to be unique within the scope
     *                   of a processor. Port identifiers should only contain alpha numeric
     *                   characters, "-" and "_".
     * @param help       The help should describe what data the port expects/produces and how it is used.
     * @see Processor::addPort()
     */
    Port(std::string_view identifier, Document help);

    void setProcessor(Processor* processor);

    std::string identifier_;
    Processor* processor_;  //< non-owning reference
    Document help_;
};

}  // namespace inviwo
