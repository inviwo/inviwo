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

#ifndef IVW_RESOURCE_H
#define IVW_RESOURCE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <string>

namespace inviwo {

/** \class Resource
 * This is an abstract class for Resources.
 * A Resource is a container for data. The data is owned by the Resource and deleted upon Resource destruction.
 * Resource implementations must implement the == operator for both other resources and a string identifier.
 * The string identifier is used to request resources from the resource manager and should be unique.
 * @see ResourceTemplate
 * @see ResourceManager
 */
class IVW_CORE_API Resource {

public:
    Resource() {};

    /**
     * Child classes are responsible for deleting the resource.
     *
     * @return
     */
    virtual ~Resource() {};

    ///**
    // * Compare two resources.
    // *
    // * @param other Other resource object.
    // * @return True if equal, false otherwise.
    // */
    //virtual bool operator==(const Resource& other) const = 0;
    //bool operator!=(const Resource& other) const {
    //    return !(*this == other);
    //}
    /**
     * Compare a resource using an identifier.
     *
     * @param identifier Identifier for resource object.
     * @return True if equal, false otherwise.
     */
    bool operator==(const std::string& identifier) const { return identifier == getIdentifier(); };
    bool operator!=(const std::string& identifier) const {
        return !(*this == identifier);
    }

    /**
     * Get a unique identifier for the resource. For example file path can often uniquely define a resource.
     *
     * @return Unique identifier for object.
     */
    virtual const std::string& getIdentifier() const = 0;
};

} // namespace

#endif // IVW_RESOURCE_H