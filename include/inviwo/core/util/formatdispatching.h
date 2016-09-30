/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_FORMATDISPATCHING_H
#define IVW_FORMATDISPATCHING_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/exception.h>

#include <tuple>

namespace inviwo {

namespace dispatching {

class IVW_CORE_API DispatchException : public Exception {
public:
    DispatchException(const std::string &message = "",
                      ExceptionContext context = ExceptionContext());
    virtual ~DispatchException() throw() = default;
};

namespace detail {

template <typename, typename>
struct Cons;

template <typename T, typename... Args>
struct Cons<T, std::tuple<Args...>> {
    using type = std::tuple<T, Args...>;
};

template <template <class> class Predicate, typename...>
struct Filter;

template <template <class> class Predicate>
struct Filter<Predicate> {
    using type = std::tuple<>;
};

template <template <class> class Predicate, typename Head, typename... Tail>
struct Filter<Predicate, Head, Tail...> {
    using type = typename std::conditional<
        Predicate<Head>::value,
        typename Cons<Head, typename Filter<Predicate, Tail...>::type>::type,
        typename Filter<Predicate, Tail...>::type>::type;
};

template <template <class> class Predicate, typename... Args>
struct Filter<Predicate, std::tuple<Args...>> {
    using type = typename Filter<Predicate, Args...>::type;
};

template <typename Result, int B, int E, typename... Args>
struct DispatchHelper {};

template <typename Result, int B, int E, typename... Formats>
struct DispatchHelper<Result, B, E, std::tuple<Formats...>> {
    static const int M = (B + E) / 2;
    using Format = typename std::tuple_element<M, std::tuple<Formats...>>::type;

    template <typename Callable, typename... Args>
    static Result dispatch(DataFormatId id, Callable &&obj, Args &&... args) {
        if (B > E)
            throw DispatchException(
                "Format " + std::string(DataFormatBase::get(id)->getString()) + " not supported",
                IvwContextCustom("Dispatching"));

        if (id == Format::id()) {
            #ifdef _WIN32 // TODO: remove win fix when VS does the right thing...
            return (obj.operator()<Result, Format>(std::forward<Args>(args)...));
            #else
            return (obj.template operator()<Result, Format>(std::forward<Args>(args)...));
            #endif
        } else if (static_cast<int>(id) < static_cast<int>(Format::id())) {
            return DispatchHelper<Result, B, M - 1, std::tuple<Formats...>>::dispatch(
                id, std::forward<Callable>(obj), std::forward<Args>(args)...);
        } else {
            return DispatchHelper<Result, M + 1, E, std::tuple<Formats...>>::dispatch(
                id, std::forward<Callable>(obj), std::forward<Args>(args)...);
        }
    }
};

} // namespace detail

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto dispatch(DataFormatId format, Callable &&obj, Args &&... args) -> Result {
    // Has to be in order of increasing id
    using Formats = std::tuple< 
        DataFloat16,
        DataFloat32,
        DataFloat64,
        DataInt8,
        DataInt16,
        DataInt32,
        DataInt64,
        DataUInt8,
        DataUInt16,
        DataUInt32,
        DataUInt64,
        DataVec2Float16,
        DataVec2Float32,
        DataVec2Float64,
        DataVec2Int8,
        DataVec2Int16,
        DataVec2Int32,
        DataVec2Int64,
        DataVec2UInt8,
        DataVec2UInt16,
        DataVec2UInt32,
        DataVec2UInt64,
        DataVec3Float16,
        DataVec3Float32,
        DataVec3Float64,
        DataVec3Int8,
        DataVec3Int16,
        DataVec3Int32,
        DataVec3Int64,
        DataVec3UInt8,
        DataVec3UInt16,
        DataVec3UInt32,
        DataVec3UInt64,
        DataVec4Float16,
        DataVec4Float32,
        DataVec4Float64,
        DataVec4Int8,
        DataVec4Int16,
        DataVec4Int32,
        DataVec4Int64,
        DataVec4UInt8,
        DataVec4UInt16,
        DataVec4UInt32,
        DataVec4UInt64
    >;

    using FilteredFormats = typename detail::Filter<Predicate, Formats>::type;

    return detail::DispatchHelper<Result, 0, std::tuple_size<FilteredFormats>::value - 1,
                          FilteredFormats>::dispatch(format, std::forward<Callable>(obj),
                                                     std::forward<Args>(args)...);
}

namespace filter {

template <typename Format>
struct All : std::true_type {};
template <typename Format>
struct Floats : std::integral_constant<bool, Format::numtype == NumericType::Float> {};
template <typename Format>
struct Integers : std::integral_constant<bool, Format::numtype != NumericType::Float> {};
template <typename Format>
struct Vecs : std::integral_constant<bool, Format::comp >= 2> {};
template <typename Format>
struct Scalars : std::integral_constant<bool, Format::comp == 1> {};

} // namespace filter

} // namespace dispatching

} // namespace inviwo

#endif // IVW_FORMATDISPATCHING_H

