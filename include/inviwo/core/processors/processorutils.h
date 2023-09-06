/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/processors/processor.h>

#include <type_traits>
#include <string_view>
#include <optional>

namespace inviwo {

class Processor;
class ProcessorMetaData;
class InviwoModule;
class InviwoApplication;

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
        p = std::make_unique<T>(std::forward<Args>(args)..., util::getInviwoApplication());
    }

    if (p->getIdentifier().empty()) p->setIdentifier(util::stripIdentifier(name));
    if (p->getDisplayName().empty()) p->setDisplayName(name);

    setPosition(p.get(), pos);

    return p;
}

/**
 * @brief Find which module that registered a processor
 * @param processor the processor to look for
 * @param app the InviwoApplication needed to get the modules
 * @return the InviwoModule that registered the processor or nullptr if not found
 */
IVW_CORE_API InviwoModule* getProcessorModule(const Processor* processor,
                                              InviwoApplication& app);

/**
 * @brief Find which module that registered a processor
 * @param classIdentifier the class identifier of the processor to look for
 * @param app the InviwoApplication needed to get the modules
 * @return the InviwoModule that registered the processor or nullptr if not found
 */
IVW_CORE_API InviwoModule* getProcessorModule(std::string_view classIdentifier,
                                              InviwoApplication& app);

/**
 * @brief Tries to set a processor's property to a given value.
 * @tparam T Type of \p proc.
 * @tparam V Type of \p val, deduced.
 * @param proc Processor that has the target property.
 * @param identifier Identifier of the property to be set.
 * @param val Value to be set.
 * @param recursive Enable/Disable recursive search for Processor::getPropertyByIdentifier.
 * @return Reference to set property.
 * @throws Exception
 */
template <typename T, typename V>
T& trySetProperty(Processor* proc, std::string_view identifier, V&& val, bool recursive = false) {

    if (auto* p = recursive ? proc->getPropertyByIdentifier(identifier, true)
                            : proc->getPropertyByPath(identifier)) {
        if (auto* tp = dynamic_cast<T*>(p)) {
            tp->set(std::forward<V>(val));
            return *tp;
        } else {
            throw Exception(IVW_CONTEXT_CUSTOM("util::trySetProperty"),
                            "Property '{}' not of type '{}'", identifier, typeid(T).name());
        }
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("util::trySetProperty"), "Could not find property: '{}'",
                        identifier);
    }
}

/**
 * @brief Find the module identifier of a registered processor
 * @param classIdentifier the class identifier of the processor to look for
 * @param app the InviwoApplication needed to get the modules
 * @return the identifer of the module that registered the processor or nullopt if not found
 */
IVW_CORE_API std::optional<std::string> getProcessorModuleIdentifier(
    std::string_view classIdentifier, InviwoApplication& app);

}  // namespace util

}  // namespace inviwo
