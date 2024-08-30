/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/image/layerrepresentation.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/assertion.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/resourcemanager/resource.h>

#include <algorithm>

#include <glm/gtx/component_wise.hpp>
#include <span>

namespace inviwo {

/**
 * \ingroup datastructures
 */
class IVW_CORE_API LayerRAM : public LayerRepresentation {
public:
    LayerRAM* clone() const override = 0;
    virtual ~LayerRAM() = default;

    /**
     * Copy and resize the representations of this onto the target.
     */
    virtual bool copyRepresentationsTo(LayerRepresentation*) const override;

    virtual void* getData() = 0;
    virtual const void* getData() const = 0;

    // Takes ownership of data pointer
    virtual void setData(void* data, size2_t dimensions) = 0;

    // uniform getters and setters
    virtual double getAsDouble(const size2_t& pos) const = 0;
    virtual dvec2 getAsDVec2(const size2_t& pos) const = 0;
    virtual dvec3 getAsDVec3(const size2_t& pos) const = 0;
    virtual dvec4 getAsDVec4(const size2_t& pos) const = 0;

    virtual void setFromDouble(const size2_t& pos, double val) = 0;
    virtual void setFromDVec2(const size2_t& pos, dvec2 val) = 0;
    virtual void setFromDVec3(const size2_t& pos, dvec3 val) = 0;
    virtual void setFromDVec4(const size2_t& pos, dvec4 val) = 0;

    virtual double getAsNormalizedDouble(const size2_t& pos) const = 0;
    virtual dvec2 getAsNormalizedDVec2(const size2_t& pos) const = 0;
    virtual dvec3 getAsNormalizedDVec3(const size2_t& pos) const = 0;
    virtual dvec4 getAsNormalizedDVec4(const size2_t& pos) const = 0;

    virtual void setFromNormalizedDouble(const size2_t& pos, double val) = 0;
    virtual void setFromNormalizedDVec2(const size2_t& pos, dvec2 val) = 0;
    virtual void setFromNormalizedDVec3(const size2_t& pos, dvec3 val) = 0;
    virtual void setFromNormalizedDVec4(const size2_t& pos, dvec4 val) = 0;

    static size_t posToIndex(const size2_t& pos, const size2_t& dim);

    virtual std::type_index getTypeIndex() const override final;

    /**
     * Dispatch functionality to retrieve the actual underlaying LayerRamPrecision.
     * The dispatcher takes a generic lambda as argument. Code will be instantiated for all the
     * DataFormat types by default. But by suppling the template `Predicate` argument the list of
     * formats to instantiate can be filtered. Hence if one knows that only Vector types are
     * applicable there is no need to write generic code that also works for scalars.

     * Example of counting the number of elements larger then 0:
     * ```{.cpp}
     * LayerRam* layerram = ...; // of some glm vector type.
     * auto count = layerram->dispatch<size_t, dispatching::filter::Vecs>([](auto lrprecision) {
     *     using LayerType = util::PrecisionType<decltype(lrprecision)>;
     *     using ValueType = util::PrecisionValueType<decltype(lrprecision)>;
     *
     *     ValueType* data = lrprecision->getDataTyped();
     *     auto dim = lrprecision->getDimensions();
     *     return std::count_if(data, data + dim.x * dim.y,
     *                          [](auto x){return x > ValueType{0};});
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
     * it will be called with the specific LayerRAMPrecision<T> as the first argument and any
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
     *	Const overload. Callable will be called with a const LayerRamPresision<T> pointer.
     */
    template <typename Result, template <class> class Predicate = dispatching::filter::All,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) const -> Result;

protected:
    LayerRAM(LayerType type = LayerType::Color);
    LayerRAM(const LayerRAM& rhs) = default;
    LayerRAM& operator=(const LayerRAM& that) = default;
};

/**
 * \ingroup datastructures
 */
template <typename T>
class LayerRAMPrecision : public LayerRAM {
public:
    using type = T;

    explicit LayerRAMPrecision(size2_t dimensions = LayerConfig::defaultDimensions,
                               LayerType type = LayerConfig::defaultType,
                               const SwizzleMask& swizzleMask = LayerConfig::defaultSwizzleMask,
                               InterpolationType interpolation = LayerConfig::defaultInterpolation,
                               const Wrapping2D& wrap = LayerConfig::defaultWrapping);
    LayerRAMPrecision(T* data, size2_t dimensions, LayerType type = LayerConfig::defaultType,
                      const SwizzleMask& swizzleMask = LayerConfig::defaultSwizzleMask,
                      InterpolationType interpolation = LayerConfig::defaultInterpolation,
                      const Wrapping2D& wrap = LayerConfig::defaultWrapping);
    explicit LayerRAMPrecision(const LayerReprConfig& config);

    LayerRAMPrecision(const LayerRAMPrecision<T>& rhs);
    LayerRAMPrecision<T>& operator=(const LayerRAMPrecision<T>& that);
    virtual LayerRAMPrecision<T>* clone() const override;
    virtual ~LayerRAMPrecision();

    virtual const DataFormatBase* getDataFormat() const override;

    T* getDataTyped();
    const T* getDataTyped() const;

    std::span<T> getView();
    std::span<const T> getView() const;

    virtual void* getData() override;
    virtual const void* getData() const override;
    virtual void setData(void* data, size2_t dimensions) override;

    /**
     * Resize the representation to dimension. This is destructive, the data will not be
     * preserved. Use copyRepresentationsTo to update the data.
     */
    virtual void setDimensions(size2_t dimensions) override;
    const size2_t& getDimensions() const override;

    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping2D& wrapping) override;
    virtual Wrapping2D getWrapping() const override;

    virtual double getAsDouble(const size2_t& pos) const override;
    virtual dvec2 getAsDVec2(const size2_t& pos) const override;
    virtual dvec3 getAsDVec3(const size2_t& pos) const override;
    virtual dvec4 getAsDVec4(const size2_t& pos) const override;

    virtual void setFromDouble(const size2_t& pos, double val) override;
    virtual void setFromDVec2(const size2_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size2_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size2_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size2_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size2_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size2_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size2_t& pos) const override;

    virtual void setFromNormalizedDouble(const size2_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size2_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size2_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size2_t& pos, dvec4 val) override;

    virtual void updateResource(const ResourceMeta& meta) const override {
        resource::meta(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())}, meta);
    }

private:
    size2_t dimensions_;
    std::unique_ptr<T[]> data_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping2D wrapping_;
};

/**
 * Factory for layers.
 * Creates an LayerRAM with data type specified by format.
 *
 * @param dimensions of layer to create.
 * @param type of layer to create.
 * @param format of layer to create.
 * @param swizzleMask used in for the layer, defaults to RGB-alpha
 * @param interpolation method to use.
 * @param wrapping method to use.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<LayerRAM> createLayerRAM(
    const size2_t& dimensions, LayerType type, const DataFormatBase* format,
    const SwizzleMask& swizzleMask = swizzlemasks::rgba,
    InterpolationType interpolation = InterpolationType::Linear,
    const Wrapping2D& wrapping = wrapping2d::clampAll);

IVW_CORE_API std::shared_ptr<LayerRAM> createLayerRAM(const LayerReprConfig& config);

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(size2_t dimensions, LayerType type,
                                        const SwizzleMask& swizzleMask,
                                        InterpolationType interpolation, const Wrapping2D& wrapping)
    : LayerRAM(type)
    , dimensions_(dimensions)
    , data_(std::make_unique<T[]>(glm::compMul(dimensions_)))
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {
    std::fill(data_.get(), data_.get() + glm::compMul(dimensions_),
              (type == LayerType::Depth) ? T{1} : T{0});

    resource::add(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())},
                  Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                           .format = DataFormat<T>::id(),
                           .desc = "LayerRAM"});
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(T* data, size2_t dimensions, LayerType type,
                                        const SwizzleMask& swizzleMask,
                                        InterpolationType interpolation, const Wrapping2D& wrapping)
    : LayerRAM(type)
    , dimensions_(dimensions)
    , data_(data)
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {
    if (!data) {
        data_ = std::make_unique<T[]>(glm::compMul(dimensions_));
        std::fill(data_.get(), data_.get() + glm::compMul(dimensions_),
                  (type == LayerType::Depth) ? T{1} : T{0});
    }
    resource::add(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())},
                  Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                           .format = DataFormat<T>::id(),
                           .desc = "LayerRAM"});
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(const LayerReprConfig& config)
    : LayerRAMPrecision{config.dimensions.value_or(LayerConfig::defaultDimensions),
                        config.type.value_or(LayerConfig::defaultType),
                        config.swizzleMask.value_or(LayerConfig::defaultSwizzleMask),
                        config.interpolation.value_or(LayerConfig::defaultInterpolation),
                        config.wrapping.value_or(LayerConfig::defaultWrapping)} {

    if (config.format && config.format != getDataFormat()) {
        throw Exception(IVW_CONTEXT, "Creating representation with unmatched type and format");
    }
}

template <typename T>
LayerRAMPrecision<T>::LayerRAMPrecision(const LayerRAMPrecision<T>& rhs)
    : LayerRAM(rhs)
    , dimensions_(rhs.dimensions_)
    , data_(std::make_unique<T[]>(glm::compMul(dimensions_)))
    , swizzleMask_(rhs.swizzleMask_)
    , interpolation_{rhs.interpolation_}
    , wrapping_{rhs.wrapping_} {
    std::copy(rhs.getView().begin(), rhs.getView().end(), data_.get());

    resource::add(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())},
                  Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                           .format = DataFormat<T>::id(),
                           .desc = "LayerRAM"});
}

template <typename T>
LayerRAMPrecision<T>& LayerRAMPrecision<T>::operator=(const LayerRAMPrecision<T>& that) {
    if (this != &that) {
        LayerRAM::operator=(that);

        const auto dim = that.dimensions_;
        auto data = std::make_unique<T[]>(glm::compMul(dimensions_));
        std::copy(that.getView().begin(), that.getView().end(), data.get());
        data_.swap(data);

        dimensions_ = that.dimensions_;
        swizzleMask_ = that.swizzleMask_;
        interpolation_ = that.interpolation_;
        wrapping_ = that.wrapping_;
        auto old = resource::remove(resource::RAM{reinterpret_cast<std::uintptr_t>(data.get())});
        resource::add(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())},
                      Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                               .format = DataFormat<T>::id(),
                               .desc = "LayerRAM",
                               .meta = resource::getMeta(old)});
    }
    return *this;
}
template <typename T>
LayerRAMPrecision<T>::~LayerRAMPrecision() {
    resource::remove(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())});
}

template <typename T>
LayerRAMPrecision<T>* LayerRAMPrecision<T>::clone() const {
    return new LayerRAMPrecision<T>(*this);
}

template <typename T>
const DataFormatBase* LayerRAMPrecision<T>::getDataFormat() const {
    return DataFormat<T>::get();
}

template <typename T>
T* LayerRAMPrecision<T>::getDataTyped() {
    return data_.get();
}

template <typename T>
const T* LayerRAMPrecision<T>::getDataTyped() const {
    return data_.get();
}

template <typename T>
std::span<T> LayerRAMPrecision<T>::getView() {
    return std::span<T>{data_.get(), glm::compMul(dimensions_)};
}

template <typename T>
std::span<const T> LayerRAMPrecision<T>::getView() const {
    return std::span<const T>{data_.get(), glm::compMul(dimensions_)};
}

template <typename T>
void* LayerRAMPrecision<T>::getData() {
    return data_.get();
}
template <typename T>
const void* LayerRAMPrecision<T>::getData() const {
    return const_cast<const T*>(data_.get());
}

template <typename T>
void LayerRAMPrecision<T>::setData(void* d, size2_t dimensions) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);
    std::swap(dimensions_, dimensions);
}

template <typename T>
void LayerRAMPrecision<T>::setDimensions(size2_t dimensions) {
    if (dimensions != dimensions_) {
        auto data = std::make_unique<T[]>(dimensions.x * dimensions.y);
        data_.swap(data);
        std::swap(dimensions, dimensions_);

        auto old = resource::remove(resource::RAM{reinterpret_cast<std::uintptr_t>(data.get())});
        resource::add(resource::RAM{reinterpret_cast<std::uintptr_t>(data_.get())},
                      Resource{.dims = glm::size4_t{dimensions_, 0, 0},
                               .format = DataFormat<T>::id(),
                               .desc = "LayerRAM",
                               .meta = resource::getMeta(old)});
    }
}

template <typename T>
const size2_t& LayerRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
void LayerRAMPrecision<T>::setSwizzleMask(const SwizzleMask& mask) {
    swizzleMask_ = mask;
}

template <typename T>
SwizzleMask LayerRAMPrecision<T>::getSwizzleMask() const {
    return swizzleMask_;
}

template <typename T>
void LayerRAMPrecision<T>::setInterpolation(InterpolationType interpolation) {
    interpolation_ = interpolation;
}

template <typename T>
InterpolationType LayerRAMPrecision<T>::getInterpolation() const {
    return interpolation_;
}

template <typename T>
void LayerRAMPrecision<T>::setWrapping(const Wrapping2D& wrapping) {
    wrapping_ = wrapping;
}

template <typename T>
Wrapping2D LayerRAMPrecision<T>::getWrapping() const {
    return wrapping_;
}

template <typename T>
double LayerRAMPrecision<T>::getAsDouble(const size2_t& pos) const {
    return util::glm_convert<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 LayerRAMPrecision<T>::getAsDVec2(const size2_t& pos) const {
    return util::glm_convert<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 LayerRAMPrecision<T>::getAsDVec3(const size2_t& pos) const {
    return util::glm_convert<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 LayerRAMPrecision<T>::getAsDVec4(const size2_t& pos) const {
    return util::glm_convert<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDouble(const size2_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec2(const size2_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec3(const size2_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromDVec4(const size2_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
double LayerRAMPrecision<T>::getAsNormalizedDouble(const size2_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 LayerRAMPrecision<T>::getAsNormalizedDVec2(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 LayerRAMPrecision<T>::getAsNormalizedDVec3(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 LayerRAMPrecision<T>::getAsNormalizedDVec4(const size2_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDouble(const size2_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec2(const size2_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec3(const size2_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void LayerRAMPrecision<T>::setFromNormalizedDVec4(const size2_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

size_t inline LayerRAM::posToIndex(const size2_t& pos, const size2_t& dim) {
    IVW_ASSERT((pos.x < dim.x) && (pos.y < dim.y), "posToIndex: position out of bounds");
    return pos.x + (pos.y * dim.x);
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto LayerRAM::dispatch(Callable&& callable, Args&&... args) -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(),
        [this]<typename T>(Callable&& obj, Args&&... args) -> Result {
            return obj(static_cast<LayerRAMPrecision<T>*>(this), std::forward<Args>(args)...);
        },
        std::forward<Callable>(callable), std::forward<Args>(args)...);
}
template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto LayerRAM::dispatch(Callable&& callable, Args&&... args) const -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(),
        [this]<typename T>(Callable&& obj, Args&&... args) -> Result {
            return obj(static_cast<const LayerRAMPrecision<T>*>(this), std::forward<Args>(args)...);
        },
        std::forward<Callable>(callable), std::forward<Args>(args)...);
}

class IVW_CORE_API LayerRamResizer {
public:
    virtual ~LayerRamResizer() = default;

    /**
     * Copy the data from @p src to @p dst. This might involve up or down scaling if the
     * dimensions do not match. The dimensions of both @p src and @p dst will not change. This
     * is only intended to be used by LayerRAM::copyRepresentationsTo. The functionality is
     * likely to be changed and should not be depended on
     * @see CIMGLayerRamResizer LayerRAM::copyRepresentationsTo
     */
    virtual bool resize(const LayerRAM& src, LayerRAM& dst) const = 0;
};

}  // namespace inviwo
