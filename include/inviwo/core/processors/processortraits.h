/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PROCESSORTRAITS_H
#define IVW_PROCESSORTRAITS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/processors/processorinfo.h>

namespace inviwo {

namespace detail {

template <class T>
class hasProcessorInfo {
    template <class U, class = typename std::enable_if<
                           !std::is_member_pointer<decltype(&U::processorInfo_)>::value>::type>
    static std::true_type check(int);
    template <class>
    static std::false_type check(...);

public:
    static const bool value = decltype(check<T>(0))::value;
};

template <typename T, typename std::enable_if<hasProcessorInfo<T>::value, std::size_t>::type = 0>
ProcessorInfo processorInfo() {
    return T::processorInfo_;
}
template <typename T, typename std::enable_if<!hasProcessorInfo<T>::value, std::size_t>::type = 0>
ProcessorInfo processorInfo() {
    return ProcessorInfo(T::CLASS_IDENTIFIER, T::DISPLAY_NAME, T::CATEGORY, T::CODE_STATE, T::TAGS);
}

}  // namesspace

/**
 * \class ProcessorTraits
 * \brief A traits class for getting the Processor info from a processor.
 * This provides a customization point if one wants to generate the processor info dynamically,
 * by specializing the traits for your kind of processor:
 *
 * template <typename T>
 * struct processor_traits<MyProcessor<T>> {
 *    static ProcessorInfo get_processor_info() {
 *       return generateMyProcessorInfo<T>();
 *   }
 * };
 *
 * The default behaviour returns the static member processorInfo_;
 *
 */
template <typename T>
struct ProcessorTraits {
    static ProcessorInfo getProcessorInfo() { return detail::processorInfo<T>(); }
};
}  // namespace

#endif  // IVW_PROCESSORTRAITS_H
