/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/indexmapper.h>

#include <tuple>
#include <type_traits>
#include <string>
#include <string_view>

namespace inviwo {

namespace dispatching {

/**
 *	Exception thrown but the dispatcher when a format can't be found.
 */
class IVW_CORE_API DispatchException : public Exception {
public:
    using Exception::Exception;
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

template <typename Index, typename Functor, Index... Is>
constexpr auto build_array_impl(Functor&& func, std::integer_sequence<Index, Is...>) noexcept {
    return std::array { func.template operator()<Is>()... };
}

template <std::size_t N, typename Index = size_t, typename Functor>
constexpr auto build_array(Functor&& func) noexcept {
    return build_array_impl<Index>(std::forward<Functor>(func),
                                   std::make_integer_sequence<Index, N>());
}

std::string IVW_CORE_API predicateNameHelper(const char* name);

template <template <class> class Predicate>
std::string predicateName() {
    return predicateNameHelper(typeid(Predicate<DataFormat<float>>).name());
}

}  // namespace detail

/**
 * Function for dispatching on a DataFormat
 *
 * # Example
 * Create a VolumeRAMPrecision<T> with the type of T given by the runtime dataFormatId.
 * @snippet src/core/util/formatdispatching.cpp Format singleDispatch example
 *
 * @tparam Result the return type of the lambda.
 * @tparam Predicate a type that is used to filter the list of types to consider in the
 *    dispatching. The `dispatching::filter` namespace have a few standard ones predefined.
 *
 * @param format ID if for the dataformat to dispatch on
 * @param obj This should be a callable with a generic call operator taking a template argument
 *  T of the dispatch type. The callable will be called with the supplied arguments (`args`).
 *  For example `[]<typename T>() {}`. T here will be, float, double, int, ... vec2, dvec2, etc.
 * @param args Any arguments that should be passed on to the callable.
 *
 * @throws dispatching::DispatchException in the case that the format is not in
 * the list of formats after the filtering.
 */
template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto singleDispatch(DataFormatId format, Callable&& obj, Args&&... args) -> Result {
    using Formats = DefaultDataFormats;
    constexpr auto nFormats = std::tuple_size_v<Formats>;
    using Functor = Result (*)(Callable&&, Args&&...);

    static constexpr auto table =
        detail::build_array<nFormats>([]<size_t index>() constexpr -> Functor {
            using Format = std::tuple_element_t<index, Formats>;
            if constexpr (Predicate<Format>::value) {
                return [](Callable&& innerObj, Args&&... innerArgs) -> Result {
                    using T = typename Format::type;
                    return innerObj.template operator()<T>(std::forward<Args>(innerArgs)...);
                };
            } else {
                return nullptr;
            }
        });

    if (format == DataFormatId::NotSpecialized) {
        throw DispatchException("Format not specialized");
    } else if (auto fun = table[static_cast<int>(format) - 1]) {
        return fun(std::forward<Callable>(obj), std::forward<Args>(args)...);
    } else {
        const auto expected = detail::predicateName<Predicate>();
        throw DispatchException(SourceContext{},
                                "Format {} not supported, expected type matching {}", format,
                                expected);
    }
}

/**
 * Function for dispatching on two DataFormats
 *
 * # Example
 * Add two VolumeRAMPrecisions with different runtime types
 * @snippet src/core/util/formatdispatching.cpp Format doubleDispatch example
 *
 * @tparam Result the return type of the lambda.
 * @tparam Predicate1 a type that is used to filter the list of types to consider in the
 *    dispatching for the first format1.
 *    The `dispatching::filter` namespace have a few standard ones predefined.
 * @tparam Predicate2 a type that is used to filter the list of types to consider in the
 *    dispatching for the second format2.
 *    The `dispatching::filter` namespace have a few standard ones predefined.
 *
 * @param format1 ID if for the first dataformat to dispatch on
 * @param format2 ID if for the second dataformat to dispatch on
 * @param obj This should be a callable with a generic call operator taking two template arguments
 *  T1 and T2 of the dispatch types. The callable will be called with the supplied arguments
 * (`args`). For example []<typename T1, typename T2>() {}.
 * T1 and T2 here will be, float, double, int, ... vec2, dvec2, etc.
 * @param args Any arguments that should be passed on to the callable.
 *
 * @throws dispatching::DispatchException in the case that the format is not in
 * the list of formats after the filtering.
 */
template <typename Result, template <class> class Predicate1, template <class> class Predicate2,
          typename Callable, typename... Args>
auto doubleDispatch(DataFormatId format1, DataFormatId format2, Callable&& obj,
                    Args&&... args) -> Result {
    using Formats = DefaultDataFormats;
    constexpr auto nFormats = std::tuple_size_v<Formats>;
    using Functor = Result (*)(Callable&&, Args&&...);

    static constexpr auto table = detail::build_array<nFormats>([]<size_t index1>() constexpr {
        using Format1 = std::tuple_element_t<index1, Formats>;
        using T1 = typename Format1::type;
        return detail::build_array<nFormats>([]<size_t index2>() constexpr -> Functor {
            using Format2 = std::tuple_element_t<index2, Formats>;
            using T2 = typename Format2::type;
            if constexpr (Predicate1<Format1>::value && Predicate2<Format2>::value) {
                return [](Callable&& innerObj, Args&&... innerArgs) -> Result {
                    return innerObj.template operator()<T1, T2>(std::forward<Args>(innerArgs)...);
                };
            } else {
                return nullptr;
            }
        });
    });

    using enum DataFormatId;
    if (format1 == NotSpecialized || format2 == NotSpecialized) {
        throw DispatchException("Format not specialized");
    } else if (auto fun =
                   table[static_cast<size_t>(format1) - 1][static_cast<size_t>(format2) - 1]) {
        return fun(std::forward<Callable>(obj), std::forward<Args>(args)...);
    } else {
        throw DispatchException(SourceContext{}, "Format combination {}, {}, not supported",
                                format1, format2);
    }
}

/**
 * Function for dispatching on three DataFormats
 *
 * # Example
 * Add two VolumeRAMPrecisions with different runtime types and a runtime result type
 * @snippet src/core/util/formatdispatching.cpp Format tripleDispatch example
 *
 * @tparam Result the return type of the lambda.
 * @tparam Predicate1 a type that is used to filter the list of types to consider in the
 *    dispatching for the first format1.
 *    The `dispatching::filter` namespace have a few standard ones predefined.
 * @tparam Predicate2 a type that is used to filter the list of types to consider in the
 *    dispatching for the second format2.
 *    The `dispatching::filter` namespace have a few standard ones predefined.
 * @tparam Predicate3 a type that is used to filter the list of types to consider in the
 *    dispatching for the third format3.
 *    The `dispatching::filter` namespace have a few standard ones predefined.
 *
 * @param format1 ID if for the first dataformat to dispatch on
 * @param format2 ID if for the second dataformat to dispatch on
 * @param format3 ID if for the third dataformat to dispatch on
 * @param obj This should be a callable with a generic call operator taking three template arguments
 *  T1, T2 and T3 of the dispatch types. The callable will be called with the supplied arguments
 * (`args`). For example []<typename T1, typename T2, typename T3>() {}.
 * T1 and T2 here will be, float, double, int, ... vec2, dvec2, etc.
 * @param args Any arguments that should be passed on to the callable.
 *
 * @throws dispatching::DispatchException in the case that the format is not in
 * the list of formats after the filtering.
 */
template <typename Result, template <class> class Predicate1, template <class> class Predicate2,
          template <class> class Predicate3, typename Callable, typename... Args>
auto tripleDispatch(DataFormatId format1, DataFormatId format2, DataFormatId format3,
                    Callable&& obj, Args&&... args) -> Result {
    using Formats = DefaultDataFormats;
    constexpr auto nFormats = std::tuple_size_v<Formats>;
    using Functor = Result (*)(Callable&&, Args&&...);

    static constexpr auto table = detail::build_array<nFormats>([]<size_t index1>() constexpr {
        using Format1 = std::tuple_element_t<index1, Formats>;
        using T1 = typename Format1::type;
        return detail::build_array<nFormats>([]<size_t index2>() constexpr {
            using Format2 = std::tuple_element_t<index2, Formats>;
            using T2 = typename Format2::type;
            return detail::build_array<nFormats>([]<size_t index3>() constexpr -> Functor {
                using Format3 = std::tuple_element_t<index3, Formats>;
                using T3 = typename Format3::type;
                if constexpr (Predicate1<Format1>::value && Predicate2<Format2>::value &&
                              Predicate3<Format3>::value) {
                    return [](Callable&& innerObj, Args&&... innerArgs) -> Result {
                        return innerObj.template operator()<T1, T2, T3>(
                            std::forward<Args>(innerArgs)...);
                    };
                } else {
                    return nullptr;
                }
            });
        });
    });

    using enum DataFormatId;
    if (format1 == NotSpecialized || format2 == NotSpecialized || format3 == NotSpecialized) {
        throw DispatchException("Format not specialized");
    } else if (auto fun = table[static_cast<size_t>(format1) - 1][static_cast<size_t>(format2) - 1]
                               [static_cast<size_t>(format3) - 1]) {
        return fun(std::forward<Callable>(obj), std::forward<Args>(args)...);
    } else {
        throw DispatchException(SourceContext{}, "Format combination {}, {}, {} not supported",
                                format1, format2, format3);
    }
}

/**
 * Same functionality as singleDispatch, but the callable is passed both the dispatch type and
 * the return type as template arguments. And the dispatch type is passed as a DataFormat<T> where
 * as singleDispatch passes T directly.
 * @see singleDispatch
 */
template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
[[deprecated("Use singleDispatch")]] auto dispatch(DataFormatId format, Callable&& obj,
                                                   Args&&... args) -> Result {
    using Formats = DefaultDataFormats;
    constexpr auto nFormats = std::tuple_size<Formats>::value;
    using Functor = Result (*)(Callable&&, Args&&...);
    static constexpr auto table =
        detail::build_array<nFormats>([]<size_t index>() constexpr -> Functor {
            using Format = std::tuple_element_t<index, Formats>;
            if constexpr (Predicate<Format>::value) {
                return [](Callable&& innerObj, Args&&... innerArgs) -> Result {
                    return innerObj.template operator()<Result, Format>(
                        std::forward<Args>(innerArgs)...);
                };
            } else {
                return nullptr;
            }
        });
    if (format == DataFormatId::NotSpecialized) {
        throw DispatchException("Format not specialized");
    } else if (auto fun = table[static_cast<int>(format) - 1]) {
        return fun(std::forward<Callable>(obj), std::forward<Args>(args)...);
    } else {
        const auto expected = detail::predicateName<Predicate>();
        throw DispatchException(SourceContext{},
                                "Format {} not supported, expected type matching {}", format,
                                expected);
    }
}

/**
 *	Namespace with standard DataFormat type filters.
 */
namespace filter {
/**
 *	Default filters matches all types.
 */
template <typename Format>
// struct All : std::true_type {};
struct All : std::integral_constant<bool, Format::rank() < 2> {};

template <typename Format>
struct ScalarAndVecs : std::integral_constant<bool, Format::rank() < 2> {};

/**
 *	Matches all floating point types. float, double, half, vec2, dvec3,...
 */
template <typename Format>
struct Floats
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype == NumericType::Float> {};

/**
 *	Matches all floating point scalar types.
 */
template <typename Format>
struct Float1s
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype == NumericType::Float &&
                                       Format::comp == 1> {};
/**
 *	Matches all floating point glm vectors types of length 2.
 */

template <typename Format>
struct Float2s
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype == NumericType::Float &&
                                       Format::comp == 2> {};

/**
 *	Matches all floating point glm vectors types of length 3.
 */
template <typename Format>
struct Float3s
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype == NumericType::Float &&
                                       Format::comp == 3> {};

/**
 *	Matches all floating point glm vectors types of length 4.
 */
template <typename Format>
struct Float4s
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype == NumericType::Float &&
                                       Format::comp == 4> {};

/**
 *	Matches all integer types, i.e. int, ivec2, uvec3...
 */
template <typename Format>
struct Integers
    : std::integral_constant<bool, Format::rank() < 2 && Format::numtype != NumericType::Float> {};

/**
 *	Matches all scalar types, i.e. int, char, long...
 */
template <typename Format>
struct Scalars : std::integral_constant<bool, Format::rank() < 2 && Format::comp == 1> {};

/**
 *	Matches all glm vector types, i.e. vec3, ivec3, uvec4,...
 */
template <typename Format>
struct Vecs : std::integral_constant<bool, Format::rank() < 2 && Format::comp >= 2> {};

/**
 * Matches all glm vector types of length 2.
 */
template <typename Format>
struct Vec2s : std::integral_constant<bool, Format::rank() < 2 && Format::comp == 2> {};

/**
 * Matches all glm vector types of length 3.
 */
template <typename Format>
struct Vec3s : std::integral_constant<bool, Format::rank() < 2 && Format::comp == 3> {};

/**
 * Matches all glm vector types of length 4.
 */
template <typename Format>
struct Vec4s : std::integral_constant<bool, Format::rank() < 2 && Format::comp == 4> {};

/**
 * Match all scalar signed or unsigned integer types
 */
template <typename Format>
struct IntegerScalars : std::integral_constant<bool, (Format::rank() < 2 && Format::comp == 1 &&
                                                      Format::numtype != NumericType::Float)> {};

/**
 * Match all scalar signed integer types
 */
template <typename Format>
struct SignedIntegerScalars
    : std::integral_constant<bool, (Format::rank() < 2 && Format::comp == 1 &&
                                    Format::numtype == NumericType::SignedInteger)> {};

/**
 * Match all scalar unsigned integer types
 */
template <typename Format>
struct UnsignedIntegerScalars
    : std::integral_constant<bool, (Format::rank() < 2 && Format::comp == 1 &&
                                    Format::numtype == NumericType::UnsignedInteger)> {};

/**
 * Match all scalar floating point types
 */
template <typename Format>
struct FloatScalars : std::integral_constant<bool, (Format::rank() < 2 && Format::comp == 1 &&
                                                    Format::numtype == NumericType::Float)> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct IntegerVecs : std::integral_constant<bool, (Format::rank() < 2 && Format::comp >= 2 &&
                                                   Format::numtype != NumericType::Float)> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct SignedIntegerVecs
    : std::integral_constant<bool, (Format::rank() < 2 && Format::comp >= 2 &&
                                    Format::numtype == NumericType::SignedInteger)> {};

/**
 * Match all signed or unsigned integer vector types
 */
template <typename Format>
struct UnsignedIntegerVecs
    : std::integral_constant<bool, (Format::rank() < 2 && Format::comp >= 2 &&
                                    Format::numtype == NumericType::UnsignedInteger)> {};

/**
 * Match all floating point vector types
 */
template <typename Format>
struct FloatVecs : std::integral_constant<bool, (Format::rank() < 2 && Format::comp >= 2 &&
                                                 Format::numtype == NumericType::Float)> {};

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
using PrecsionType [[deprecated("Use `PrecisionType` instead")]] =
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
using PrecsionValueType [[deprecated("Use `PrecisionValueType` instead")]] =
    typename PrecisionType<T>::type;

}  // namespace util

}  // namespace inviwo
