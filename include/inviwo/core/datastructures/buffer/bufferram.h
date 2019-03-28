/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_BUFFER_RAM_H
#define IVW_BUFFER_RAM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/formatdispatching.h>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API BufferRAM : public BufferRepresentation {
public:
    BufferRAM(const DataFormatBase* format, BufferUsage usage = BufferUsage::Static,
              BufferTarget target = BufferTarget::Data);
    BufferRAM(const BufferRAM& rhs) = default;
    BufferRAM& operator=(const BufferRAM& that) = default;
    virtual BufferRAM* clone() const override = 0;
    virtual ~BufferRAM() = default;

    virtual void* getData() = 0;
    virtual const void* getData() const = 0;

    virtual void reserve(size_t size) = 0;
    virtual void clear() = 0;

    // uniform getters and setters
    virtual double getAsDouble(const size_t& pos) const = 0;
    virtual dvec2 getAsDVec2(const size_t& pos) const = 0;
    virtual dvec3 getAsDVec3(const size_t& pos) const = 0;
    virtual dvec4 getAsDVec4(const size_t& pos) const = 0;

    virtual void setFromDouble(const size_t& pos, double val) = 0;
    virtual void setFromDVec2(const size_t& pos, dvec2 val) = 0;
    virtual void setFromDVec3(const size_t& pos, dvec3 val) = 0;
    virtual void setFromDVec4(const size_t& pos, dvec4 val) = 0;

    virtual double getAsNormalizedDouble(const size_t& pos) const = 0;
    virtual dvec2 getAsNormalizedDVec2(const size_t& pos) const = 0;
    virtual dvec3 getAsNormalizedDVec3(const size_t& pos) const = 0;
    virtual dvec4 getAsNormalizedDVec4(const size_t& pos) const = 0;

    virtual void setFromNormalizedDouble(const size_t& pos, double val) = 0;
    virtual void setFromNormalizedDVec2(const size_t& pos, dvec2 val) = 0;
    virtual void setFromNormalizedDVec3(const size_t& pos, dvec3 val) = 0;
    virtual void setFromNormalizedDVec4(const size_t& pos, dvec4 val) = 0;

    virtual std::type_index getTypeIndex() const override final;

    /**
     * Dispatch functionality to retrieve the actual underlaying BufferRamPrecision.
     * The dispatcher takes a generic lambda as argument. Code will be instantiated for all the
     * DataFormat types by default. But by suppling the template `Predicate` argument the list of
     * formats to instantiate can be filtered. Hence if one knows that only Vector types are
     * applicable there is no need to write generic code that also works for scalars.

     * Example of counting the number of elements larger then 0:
     * ```{.cpp}
     * BufferRam* bufferram = ...; // of some glm vector type.
     * auto count = bufferram->dispatch<size_t, dispatching::filter::Vecs>([](auto brprecision) {
     *     using BufferType = util::PrecisionType<decltype(brprecision)>;
     *     using ValueType = util::PrecisionValueType<decltype(brprecision)>;
     *
     *     std::vector<ValueType>& data = brprecision->getDataContainer();
     *     return std::count_if(data.begin(), data.end(), [](auto x){return x > ValueType{0};});
     * });
     *
     * ```
     *
     * # Template arguments:
     *  * __Result__ the return type of the lambda.
     *  * __Predicate__ A type that is used to filter the list of types to consider in the
     *    dispatching. The `dispatching::filter` namespace have a few standard ones predefined.
     *
     * # Predicates:
     *  * __All__ Matches all formats, default.
     *  * __Floats__ Matches all floating point types. float, double, half, vec2, dvec3,...
     *  * __Integers__ Matches all integer types, i.e. int, ivec2, uvec3...
     *  * __Scalars__ Matches all scalar types, i.e. int, char, long, float, ...
     *  * __Vecs__ Matches all glm vector types, i.e. vec3, ivec3, uvec4,...
     *  * __VecNs__ Matches all glm vector types of length N. N = 2,3,4.
     *  * __FloatNs__ Matches all floating point glm vector types of length N. N = 2,3,4.
     *
     * @param callable This should be a generic lambda or a struct with a generic call operator.
     * it will be called with the specific BufferRamPresision<T> as the first argument and any
     * additional arguments (`args`) appended to that.
     * @param args Any additional arguments that should be passed on to the lambda.
     *
     * @throws dispatching::DispatchException in the case that the format of the buffer is not in
     * the list of formats after the filtering.
     */
    template <typename Result, template <class> class Predicate = dispatching::filter::All,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) -> Result;

    /**
     *	Const overload. Callable will be called with a const BufferRamPresision<T> pointer.
     */
    template <typename Result, template <class> class Predicate = dispatching::filter::All,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) const -> Result;
};

template <typename T, BufferTarget Target>
class BufferRAMPrecision;

namespace detail {
struct BufferRamDispatcher {
    template <typename Result, typename Format, typename Callable, typename... Args>
    Result operator()(Callable&& obj, BufferRAM* bufferram, Args... args) {
        switch (bufferram->getBufferTarget()) {
            case BufferTarget::Index:
                return obj(
                    static_cast<BufferRAMPrecision<typename Format::type, BufferTarget::Index>*>(
                        bufferram),
                    std::forward<Args>(args)...);
            case BufferTarget::Data:
            default:
                return obj(
                    static_cast<BufferRAMPrecision<typename Format::type, BufferTarget::Data>*>(
                        bufferram),
                    std::forward<Args>(args)...);
        }
    }
};

struct BufferRamConstDispatcher {
    template <typename Result, typename Format, typename Callable, typename... Args>
    Result operator()(Callable&& obj, const BufferRAM* bufferram, Args... args) {
        switch (bufferram->getBufferTarget()) {
            case BufferTarget::Index:
                return obj(static_cast<const BufferRAMPrecision<typename Format::type,
                                                                BufferTarget::Index>*>(bufferram),
                           std::forward<Args>(args)...);

            case BufferTarget::Data:
            default:
                return obj(static_cast<const BufferRAMPrecision<typename Format::type,
                                                                BufferTarget::Data>*>(bufferram),
                           std::forward<Args>(args)...);
        }
    }
};
}  // namespace detail

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto BufferRAM::dispatch(Callable&& callable, Args&&... args) -> Result {
    detail::BufferRamDispatcher dispatcher;
    return dispatching::dispatch<Result, Predicate>(getDataFormatId(), dispatcher,
                                                    std::forward<Callable>(callable), this,
                                                    std::forward<Args>(args)...);
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto BufferRAM::dispatch(Callable&& callable, Args&&... args) const -> Result {
    detail::BufferRamConstDispatcher dispatcher;
    return dispatching::dispatch<Result, Predicate>(getDataFormatId(), dispatcher,
                                                    std::forward<Callable>(callable), this,
                                                    std::forward<Args>(args)...);
}

/**
 * Factory for buffers.
 * Creates a BufferRAM with data type specified by format.
 *
 * @param size of buffer to create.
 * @param format of buffer to create.
 * @param usage   usage of the buffer (static or dynamic)
 * @param target  target of the buffer (index or data)
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<BufferRAM> createBufferRAM(size_t size, const DataFormatBase* format,
                                                        BufferUsage usage,
                                                        BufferTarget target = BufferTarget::Data);

template <BufferUsage U = BufferUsage::Static, typename T = vec3,
          BufferTarget Target = BufferTarget::Data>
std::shared_ptr<BufferRAMPrecision<T, Target>> createBufferRAM(std::vector<T> data) {
    return std::make_shared<BufferRAMPrecision<T, Target>>(std::move(data), DataFormat<T>::get(),
                                                           U);
}

/**
 * \brief compare two buffers using their RAM representation
 *
 * @return true if buffers are identical, i.e. identical data format, size, and buffer contents
 */
bool IVW_CORE_API operator==(const BufferBase& bufA, const BufferBase& bufB);
bool IVW_CORE_API operator!=(const BufferBase& bufA, const BufferBase& bufB);

}  // namespace inviwo

#endif  // IVW_BUFFER_RAM_H
