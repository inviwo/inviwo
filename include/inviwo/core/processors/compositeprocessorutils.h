/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_COMPOSITEPROCESSORUTILS_H
#define IVW_COMPOSITEPROCESSORUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class ProcessorNetwork;
class CompositeProcessor;

namespace util {

/**
 * Create a CompositeProcessor out of the currently selected processors and replace them with the
 * composite processors. The selected processors are moved from the current network into the sub
 * network of the composite processor. For each port connection between a selected and unselected
 * processor a composite sink or composite source processor is added to the sub network and
 * connections are made from the selected processor to the sink/source and from the composite
 * processor to the unselected processor. For each link between a selected and unselected processor,
 * a super property is added to the composite processor and the link added to it.
 */
IVW_CORE_API void replaceSelectionWithCompositeProcessor(ProcessorNetwork& network);

/**
 * Expand a composite processors sub network into its network. Effectively reversing the actions of
 * replaceSelectionWithCompositeProcessor. All processor except for composite sink and composite
 * source processors are moved from the sub network into the network of the composite processor.
 * Connections and links are the reestablished. Sources and sinks are discarded.
 */
IVW_CORE_API void expandCompositeProcessorIntoNetwork(CompositeProcessor& composite);

}  // namespace util

}  // namespace inviwo

#endif  // IVW_COMPOSITEPROCESSORUTILS_H
