/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/buffer/bufferrepresentation.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/glmconvert.h>
#include <inviwo/core/util/stdextensions.h>

#include <initializer_list>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API BufferRAM : public BufferRepresentation {
public:
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

protected:
    BufferRAM(BufferUsage usage = BufferUsage::Static, BufferTarget target = BufferTarget::Data);
    BufferRAM(const BufferRAM& rhs) = default;
    BufferRAM& operator=(const BufferRAM& that) = default;
};

/**
 * \ingroup datastructures
 */
template <typename T, BufferTarget Target = BufferTarget::Data>
class BufferRAMPrecision : public BufferRAM {
public:
    using type = T;
    static const BufferTarget target = Target;

    explicit BufferRAMPrecision(BufferUsage usage = BufferUsage::Static);
    explicit BufferRAMPrecision(size_t size, BufferUsage usage = BufferUsage::Static);
    explicit BufferRAMPrecision(std::vector<T> data, BufferUsage usage = BufferUsage::Static);
    BufferRAMPrecision(const BufferRAMPrecision<T, Target>& rhs) = default;
    BufferRAMPrecision<T, Target>& operator=(const BufferRAMPrecision<T, Target>& that) = default;
    virtual ~BufferRAMPrecision() = default;
    virtual BufferRAMPrecision<T, Target>* clone() const override;

    virtual const DataFormatBase* getDataFormat() const override;

    virtual void setSize(size_t size) override;
    virtual size_t getSize() const override;

    virtual void* getData() override;
    virtual const void* getData() const override;
    std::vector<T>& getDataContainer();
    const std::vector<T>& getDataContainer() const;

    virtual void reserve(size_t size) override;

    virtual double getAsDouble(const size_t& pos) const override;
    virtual dvec2 getAsDVec2(const size_t& pos) const override;
    virtual dvec3 getAsDVec3(const size_t& pos) const override;
    virtual dvec4 getAsDVec4(const size_t& pos) const override;

    virtual void setFromDouble(const size_t& pos, double val) override;
    virtual void setFromDVec2(const size_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size_t& pos) const override;

    virtual void setFromNormalizedDouble(const size_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size_t& pos, dvec4 val) override;

    void add(const T& item);
    void add(std::initializer_list<T> data);
    void append(const std::vector<T>* data);
    void append(const std::vector<T>& data);

    T& operator[](size_t i);
    const T& operator[](size_t i) const;

    void set(size_t index, const T& item);
    T get(size_t index) const;
    T& get(size_t index);

    virtual void clear() override;

private:
    std::vector<T> data_;
};

using FloatBufferRAM = BufferRAMPrecision<float>;
using Vec2BufferRAM = BufferRAMPrecision<vec2>;
using Vec3BufferRAM = BufferRAMPrecision<vec3>;
using Vec4BufferRAM = BufferRAMPrecision<vec4>;
// Used for index buffers
using IndexBufferRAM = BufferRAMPrecision<std::uint32_t, BufferTarget::Index>;

template <typename T, BufferTarget Target>
const T& BufferRAMPrecision<T, Target>::operator[](size_t i) const {
    return data_[i];
}

template <typename T, BufferTarget Target>
T& BufferRAMPrecision<T, Target>::operator[](size_t i) {
    return data_[i];
}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>::BufferRAMPrecision(BufferUsage usage)
    : BufferRAMPrecision(0, usage) {}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>::BufferRAMPrecision(size_t size, BufferUsage usage)
    : BufferRAM(usage, Target), data_(size) {}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>::BufferRAMPrecision(std::vector<T> data, BufferUsage usage)
    : BufferRAM(usage, Target), data_(std::move(data)) {}

template <typename T, BufferTarget Target>
BufferRAMPrecision<T, Target>* BufferRAMPrecision<T, Target>::clone() const {
    return new BufferRAMPrecision<T, Target>(*this);
}
template <typename T, BufferTarget Target>
const DataFormatBase* BufferRAMPrecision<T, Target>::getDataFormat() const {
    return DataFormat<T>::get();
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setSize(size_t size) {
    return data_.resize(size);
}

template <typename T, BufferTarget Target>
size_t BufferRAMPrecision<T, Target>::getSize() const {
    return data_.size();
}

template <typename T, BufferTarget Target>
void* BufferRAMPrecision<T, Target>::getData() {
    return (data_.empty() ? nullptr : data_.data());
}

template <typename T, BufferTarget Target>
const void* BufferRAMPrecision<T, Target>::getData() const {
    return (data_.empty() ? nullptr : data_.data());
}

template <typename T, BufferTarget Target>
std::vector<T>& BufferRAMPrecision<T, Target>::getDataContainer() {
    return data_;
}

template <typename T, BufferTarget Target>
const std::vector<T>& BufferRAMPrecision<T, Target>::getDataContainer() const {
    return data_;
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::reserve(size_t size) {
    data_.reserve(size);
}

template <typename T, BufferTarget Target>
double BufferRAMPrecision<T, Target>::getAsDouble(const size_t& pos) const {
    return util::glm_convert<double>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec2 BufferRAMPrecision<T, Target>::getAsDVec2(const size_t& pos) const {
    return util::glm_convert<dvec2>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec3 BufferRAMPrecision<T, Target>::getAsDVec3(const size_t& pos) const {
    return util::glm_convert<dvec3>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec4 BufferRAMPrecision<T, Target>::getAsDVec4(const size_t& pos) const {
    return util::glm_convert<dvec4>(data_[pos]);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDouble(const size_t& pos, double val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec2(const size_t& pos, dvec2 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec3(const size_t& pos, dvec3 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromDVec4(const size_t& pos, dvec4 val) {
    data_[pos] = util::glm_convert<T>(val);
}

template <typename T, BufferTarget Target>
double BufferRAMPrecision<T, Target>::getAsNormalizedDouble(const size_t& pos) const {
    return util::glm_convert_normalized<double>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec2 BufferRAMPrecision<T, Target>::getAsNormalizedDVec2(const size_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec3 BufferRAMPrecision<T, Target>::getAsNormalizedDVec3(const size_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[pos]);
}

template <typename T, BufferTarget Target>
dvec4 BufferRAMPrecision<T, Target>::getAsNormalizedDVec4(const size_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[pos]);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDouble(const size_t& pos, double val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec2(const size_t& pos, dvec2 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec3(const size_t& pos, dvec3 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::setFromNormalizedDVec4(const size_t& pos, dvec4 val) {
    data_[pos] = util::glm_convert_normalized<T>(val);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::add(const T& item) {
    data_.push_back(item);
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::add(std::initializer_list<T> data) {
    for (auto& elem : data) {
        data_.push_back(elem);
    }
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::append(const std::vector<T>* data) {
    data_.insert(data_.end(), data->begin(), data->end());
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::append(const std::vector<T>& data) {
    data_.insert(data_.end(), data.begin(), data.end());
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::set(size_t index, const T& item) {
    data_[index] = item;
}

template <typename T, BufferTarget Target>
T BufferRAMPrecision<T, Target>::get(size_t index) const {
    return data_[index];
}

template <typename T, BufferTarget Target>
T& BufferRAMPrecision<T, Target>::get(size_t index) {
    return data_[index];
}

template <typename T, BufferTarget Target>
void BufferRAMPrecision<T, Target>::clear() {
    data_.clear();
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto BufferRAM::dispatch(Callable&& callable, Args&&... args) -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(), [&]<typename T>() -> Result {
            switch (getBufferTarget()) {
                case BufferTarget::Index:
                    return callable(static_cast<BufferRAMPrecision<T, BufferTarget::Index>*>(this),
                                    std::forward<Args>(args)...);
                case BufferTarget::Data:
                default:
                    return callable(static_cast<BufferRAMPrecision<T, BufferTarget::Data>*>(this),
                                    std::forward<Args>(args)...);
            }
        });
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto BufferRAM::dispatch(Callable&& callable, Args&&... args) const -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(), [&]<typename T>() -> Result {
            switch (getBufferTarget()) {
                case BufferTarget::Index:
                    return callable(
                        static_cast<const BufferRAMPrecision<T, BufferTarget::Index>*>(this),
                        std::forward<Args>(args)...);
                case BufferTarget::Data:
                default:
                    return callable(
                        static_cast<const BufferRAMPrecision<T, BufferTarget::Data>*>(this),
                        std::forward<Args>(args)...);
            }
        });
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
