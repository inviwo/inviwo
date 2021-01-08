/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2021 Inviwo Foundation
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

#include <utility>

namespace inviwo {

class Processor;
class PropertyOwner;
class Property;
class CompositeProperty;
class CanvasProcessor;

/**
 * @brief Visitor pattern base for visiting an Inviwo ProcessorNetwork
 */
class IVW_CORE_API NetworkVisitor {
public:
    virtual ~NetworkVisitor() = default;

    /**
     * @brief Visit a Processor
     * Adding and removing processors while visiting are not supported
     * @return visit all child properties if true else go to next processor
     */
    virtual bool visit(Processor&) { return true; }

    /**
     * @brief Visit a CanvasProcessor
     * Adding and removing processors while visiting are not supported
     * @return visit all child properties if true else go to next processor
     */
    virtual bool visit(CanvasProcessor&) { return true; }

    /**
     * @brief Visit a CompositeProperty
     * Adding and removing properties while visiting are not supported
     * @return visit all child properties if true else go to next CompositeProperty
     */
    virtual bool visit(CompositeProperty&) { return true; }

    /**
     * @brief Visit a Property
     * Adding and removing properties while visiting are not supported
     * @return not used for Properties
     */
    virtual bool visit(Property&) { return true; }
};

}  // namespace inviwo
