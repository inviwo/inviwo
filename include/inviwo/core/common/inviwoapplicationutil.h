/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <cstddef>

namespace inviwo {

class InviwoApplication;
class ProcessorNetwork;
class Processor;
class PropertyOwner;
class Property;

namespace util {

/**
 * Utility function to get the InviwoApplication from a ProcessorNetwork
 */
IVW_CORE_API InviwoApplication* getInviwoApplication(ProcessorNetwork*);
/**
 * Utility function to get the InviwoApplication from a Processor
 */
IVW_CORE_API InviwoApplication* getInviwoApplication(Processor*);
/**
 * Utility function to get the InviwoApplication from a PropertyOwner
 */
IVW_CORE_API InviwoApplication* getInviwoApplication(PropertyOwner*);
/**
 * Utility function to get the InviwoApplication from a Property
 */
IVW_CORE_API InviwoApplication* getInviwoApplication(Property*);
/**
 * Utility function to get the InviwoApplication
 */
IVW_CORE_API InviwoApplication* getInviwoApplication();

/**
 * Utility function to return whether the InviwoApplication is initialized
 * @see InviwoApplication::isInitialized
 */
IVW_CORE_API bool isInviwoApplicationInitialized();

}  // namespace util

}  // namespace inviwo
