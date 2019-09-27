/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_PROCESSORUTILS_H
#define IVW_PROCESSORUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processortraits.h>

#include <type_traits>

namespace inviwo {

class Processor;
class ProcessorMetaData;

namespace util {

/**
 * Retrieve the meta data of the processor.
 */
IVW_CORE_API const ProcessorMetaData* getMetaData(const Processor* processor);

/**
 * Retrieve the meta data of the processor.
 */
IVW_CORE_API ProcessorMetaData* getMetaData(Processor* processor);

/**
 * Retrieve the position of the processor.
 */
IVW_CORE_API ivec2 getPosition(const Processor* processor);

/**
 * Set the position of processor to pos
 */
IVW_CORE_API void setPosition(Processor* processor, ivec2 pos);

/**
 * Retrieve the selection state of processor.
 */
IVW_CORE_API bool isSelected(const Processor* processor);

/**
 * Set the selection state of processor
 */
IVW_CORE_API void setSelected(Processor* processor, bool selected);

/**
 * A utility class to place processors on the "grid" in the editor
 */
struct IVW_CORE_API GridPos {
    GridPos(int x, int y) : pos_{x, y} {};
    explicit GridPos(ivec2 pos) : pos_{pos} {}

    operator ivec2() const { return pos_ * ivec2{25, 25}; }

private:
    ivec2 pos_;
};

/**
 * A utility function to create a processor and set identifier, display name, and position
 * @param pos Sets the position meta data of the Processor
 * @param args Any extra arguments to supply to the Processor constructor
 */
template <typename T, typename... Args>
std::unique_ptr<T> makeProcessor(ivec2 pos, Args&&... args) {
    auto name = ProcessorTraits<T>::getProcessorInfo().displayName;

    std::unique_ptr<T> p;

    if constexpr (std::is_constructible_v<T, Args...>) {
        p = std::make_unique<T>(std::forward<Args>(args)...);
    } else {
        p = std::make_unique<T>(std::forward<Args>(args)..., InviwoApplication::getPtr());
    }

    if (p->getIdentifier().empty()) p->setIdentifier(name);
    if (p->getDisplayName().empty()) p->setDisplayName(name);

    setPosition(p.get(), pos);

    return std::move(p);
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_PROCESSORUTILS_H
