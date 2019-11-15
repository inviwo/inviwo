/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <type_traits>

namespace inviwo {

namespace dispatching {

/**
 *	Exception thrown but the dispatcher when a format can't be found.
 */
class IVW_CORE_API DispatchException : public Exception {
public:
    DispatchException(const std::string &message = "",
                      ExceptionContext context = ExceptionContext());
    virtual ~DispatchException() throw() = default;
};

namespace detail {

/**
 * Helper class to add types to a tuple.
 */
template <typename, typename>
struct Cons;

template <typename T, typename... Args>
struct Cons<T, std::tuple<Args...>> {
    using type = std::tuple<T, Args...>;
};

/**
 *	Helper class to filter a list ot types based on a predicate
 */
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

/**
 * Helper class to find the matching DataFormatId among a sorted list of types.
 * Does a binary search in the type list.
 */
template <typename Result, int B, int E, typename... Args>
struct DispatchHelper {};

#include <warn/push>
#include <warn/ignore/constant-conditional>

template <typename Result, int B, int E, typename... Formats>
struct DispatchHelper<Result, B, E, std::tuple<Formats...>> {
    static const int M = (B + E) / 2;
    using Format = typename std::tuple_element<M, std::tuple<Formats...>>::type;

    template <typename Callable, typename... Args>
    static Result dispatch(DataFormatId id, Callable &&obj, Args &&... args) {
        if (B > E)
            throw DispatchException(
                "Format " + std::string(DataFormatBase::get(id)->getString()) + " not supported",
                IVW_CONTEXT_CUSTOM("Dispatching"));

        if (id == Format::id()) {
#ifdef _WIN32  // TODO: remove win fix when VS does the right thing...
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

#include <warn/pop>

}  // namespace detail

/**
 * Function for dispatching a DataFormat based on the DataFormatId.
 * The matching DataFormat is found using binary search in the type list.
 *
 * # Template arguments:
 *  * __Result__ the return type of the lambda.
 *  * __Predicate__ A type that is used to filter the list of types to consider in the
 *    dispatching. The `dispatching::filter` namespace have a few standard ones predefined.
 *
 * # Example
 * \snippet src/core/datastructures/buffer/bufferram.cpp Format Dispatching Example
 *
 * @param format ID if for the dataformat to dispatch on
 * @param obj This should be a struct with a generic call operator taking two template
 * arguments the result type and DataFormat type. The callable will be called with the supplied
 * arguments (`args`).
 * @param args Any arguments that should be passed on to the lambda.
 *
 *
 * @throws dispatching::DispatchException in the case that the format of the buffer is not in
 * the list of formats after the filtering.
 */
template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto dispatch(DataFormatId format, Callable &&obj, Args &&... args) -> Result {
    // Has to be in order of increasing id

    using Formats = DefaultDataFormats;

    using FilteredFormats = typename detail::Filter<Predicate, Formats>::type;

    return detail::DispatchHelper<Result, 0, std::tuple_size<FilteredFormats>::value - 1,
                                  FilteredFormats>::dispatch(format, std::forward<Callable>(obj),
                                                             std::forward<Args>(args)...);
}

/**
 *	Namespace with standard DataFormat type filters.
 */
namespace filter {

/**
 *	Default filters matches all types.
 */
template <typename Format>
struct All : std::true_type {};

/**
 *	Matches all floating point types. float, double, half, vec2, dvec3,...
 */
template <typename Format>
struct Floats : std::integral_constant<bool, Format::numtype == NumericType::Float> {};

/**
 *	Matches all floating point scalar types.
 */
template <typename Format>
struct Float1s
    : std::integral_constant<bool, Format::numtype == NumericType::Float && Format::comp == 1> {};
/**
 *	Matches all floating point glm vectors types of length 2.
 */

template <typename Format>
struct Float2s
    : std::integral_constant<bool, Format::numtype == NumericType::Float && Format::comp == 2> {};

/**
 *	Matches all floating point glm vectors types of length 3.
 */
template <typename Format>
struct Float3s
    : std::integral_constant<bool, Format::numtype == NumericType::Float && Format::comp == 3> {};

/**
 *	Matches all floating point glm vectors types of length 4.
 */
template <typename Format>
struct Float4s
    : std::integral_constant<bool, Format::numtype == NumericType::Float && Format::comp == 4> {};

/**
 *	Matches all integer types, i.e. int, ivec2, uvec3...
 */
template <typename Format>
struct Integers : std::integral_constant<bool, Format::numtype != NumericType::Float> {};

/**
 *	Matches all scalar types, i.e. int, char, long...
 */
template <typename Format>
struct Scalars : std::integral_constant<bool, Format::comp == 1> {};

/**
 *	Matches all glm vector types, i.e. vec3, ivec3, uvec4,...
 */
template <typename Format>
struct Vecs : std::integral_constant<bool, Format::comp >= 2> {};

/**
 * Matches all glm vector types of length 2.
 */
template <typename Format>
struct Vec2s : std::integral_constant<bool, Format::comp == 2> {};

/**
 * Matches all glm vector types of length 3.
 */
template <typename Format>
struct Vec3s : std::integral_constant<bool, Format::comp == 3> {};

/**
 * Matches all glm vector types of length 4.
 */
template <typename Format>
struct Vec4s : std::integral_constant<bool, Format::comp == 4> {};

/**
 * Match all scalar signed or unsigned integer types
 */
template <typename Format>
struct IntegerScalars
    : std::integral_constant<bool, Format::comp == 1 && Format::numtype != NumericType::Float> {};

/**
 * Match all scalar signed integer types
 */
template <typename Format>
struct SignedIntegerScalars
    : std::integral_constant<bool,
                             Format::comp == 1 && Format::numtype == NumericType::SignedInteger> {};

/**
 * Match all scalar unsigned integer types
 */
template <typename Format>
struct UnsignedIntegerScalars
    : std::integral_constant<bool, Format::comp == 1 &&
                                       Format::numtype == NumericType::UnsignedInteger> {};

/**
 * Match all scalar floating point types
 */
template <typename Format>
struct FloatScalars
    : std::integral_constant<bool, Format::comp == 1 && Format::numtype == NumericType::Float> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct IntegerVecs
    : std::integral_constant<bool, Format::comp >= 2 && Format::numtype != NumericType::Float> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct SignedIntegerVecs
    : std::integral_constant<bool,
                             Format::comp >= 2 && Format::numtype == NumericType::SignedInteger> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct UnsignedIntegerVecs
    : std::integral_constant<bool, Format::comp >= 2 &&
                                       Format::numtype == NumericType::UnsignedInteger> {};

/**
 * Match all floating point vector types
 */
template <typename Format>
struct FloatVecs
    : std::integral_constant<bool, Format::comp >= 2 && Format::numtype == NumericType::Float> {};

}  // namespace filter

}  // namespace dispatching

namespace util {

/**
 * Utility for retrieving the type of a (Buffer/Layer/Volume)RamPrecision pointer variable.
 * Example usage:
 * ```{.cpp}
 * VolumeRam* volumeram = ...; // of some glm vector type.
 * auto count = volumeram->dispatch<size_t, dispatching::filter::Vecs>([](auto vrprecision) {
 *     using VolumeType = util::PrecisionType<decltype(vrprecision)>;
 *     ....
 * ```
 * VolumeType will then be for example VolumeRamPrecision<vec3>
 */
template <typename T>
using PrecisionType = typename std::remove_pointer<typename std::remove_const<T>::type>::type;

template <typename T>
using PrecsionType[[deprecated("Use `PrecisionType` instead")]] =
    typename std::remove_pointer<typename std::remove_const<T>::type>::type;

/**
 * Utility for retrieving the type of a (Buffer/Layer/Volume)RamPrecision pointer variable.
 * Example usage:
 * ```{.cpp}
 * VolumeRam* volumeram = ...; // of some glm vector type.
 * auto count = volumeram->dispatch<size_t, dispatching::filter::Vecs>([](auto vrprecision) {
 *     using ValueType = util::PrecisionValueType<decltype(vrprecision)>;
 *     ....
 * ```
 * ValueType will then be for example vec3
 */
template <typename T>
using PrecisionValueType = typename PrecisionType<T>::type;

template <typename T>
using PrecsionValueType[[deprecated("Use `PrecisionValueType` instead")]] =
    typename PrecisionType<T>::type;

}  // namespace util

}  // namespace inviwo

#endif  // IVW_FORMATDISPATCHING_H
