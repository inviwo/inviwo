/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/stdextensions.h>

#include <glm/gtx/component_wise.hpp>
#include <span>

namespace inviwo {

class HistogramCalculationState;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API VolumeRAM : public VolumeRepresentation {
public:
    virtual VolumeRAM* clone() const override = 0;
    virtual ~VolumeRAM() = default;

    virtual void* getData() = 0;
    virtual const void* getData() const = 0;
    virtual void* getData(size_t) = 0;
    virtual const void* getData(size_t) const = 0;

    /**
     * \brief Takes ownership of data pointer
     *
     * @param data is raw volume data pointer
     * @param dimensions is the dimensions of the data.
     */
    virtual void setData(void* data, size3_t dimensions) = 0;
    virtual void removeDataOwnership() = 0;

    // uniform getters and setters
    virtual double getAsDouble(const size3_t& pos) const = 0;
    virtual dvec2 getAsDVec2(const size3_t& pos) const = 0;
    virtual dvec3 getAsDVec3(const size3_t& pos) const = 0;
    virtual dvec4 getAsDVec4(const size3_t& pos) const = 0;

    virtual void setFromDouble(const size3_t& pos, double val) = 0;
    virtual void setFromDVec2(const size3_t& pos, dvec2 val) = 0;
    virtual void setFromDVec3(const size3_t& pos, dvec3 val) = 0;
    virtual void setFromDVec4(const size3_t& pos, dvec4 val) = 0;

    virtual double getAsNormalizedDouble(const size3_t& pos) const = 0;
    virtual dvec2 getAsNormalizedDVec2(const size3_t& pos) const = 0;
    virtual dvec3 getAsNormalizedDVec3(const size3_t& pos) const = 0;
    virtual dvec4 getAsNormalizedDVec4(const size3_t& pos) const = 0;

    virtual void setFromNormalizedDouble(const size3_t& pos, double val) = 0;
    virtual void setFromNormalizedDVec2(const size3_t& pos, dvec2 val) = 0;
    virtual void setFromNormalizedDVec3(const size3_t& pos, dvec3 val) = 0;
    virtual void setFromNormalizedDVec4(const size3_t& pos, dvec4 val) = 0;

    virtual size_t getNumberOfBytes() const = 0;

    template <typename T>
    static T posToIndex(const glm::tvec3<T, glm::defaultp>& pos,
                        const glm::tvec3<T, glm::defaultp>& dim);
    template <typename T>
    static T periodicPosToIndex(const glm::tvec3<T, glm::defaultp>& posIn,
                                const glm::tvec3<T, glm::defaultp>& dim);

    virtual std::type_index getTypeIndex() const override final;

    /**
     * Dispatch functionality to retrieve the actual underlaying VolumeRamPrecision.
     * The dispatcher takes a generic lambda as argument. Code will be instantiated for all the
     * DataFormat types by default. But by suppling the template `Predicate` argument the list of
     * formats to instantiate can be filtered. Hence if one knows that only Vector types are
     * applicable there is no need to write generic code that also works for scalars.

     * Example of counting the number of elements larger then 0:
     * ```{.cpp}
     * VolumeRam* volumeram = ...; // of some glm vector type.
     * auto count = volumeram->dispatch<size_t, dispatching::filter::Vecs>([](auto vrprecision) {
     *     using VolumeType = util::PrecisionType<decltype(vrprecision)>;
     *     using ValueType = util::PrecisionValueType<decltype(vrprecision)>;
     *
     *     ValueType* data = vrprecision->getDataTyped();
     *     auto dim = vrprecision->getDimensions();
     *     return std::count_if(data, data + dim.x * dim.y * dim.z,
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
     * it will be called with the specific VolumeRamPresision<T> as the first argument and any
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
     *	Const overload. Callable will be called with a const VolumeRamPresision<T> pointer.
     */
    template <typename Result, template <class> class Predicate = dispatching::filter::All,
              typename Callable, typename... Args>
    auto dispatch(Callable&& callable, Args&&... args) const -> Result;

protected:
    VolumeRAM() = default;
    VolumeRAM(const VolumeRAM& rhs) = default;
    VolumeRAM(VolumeRAM& rhs) = default;
    VolumeRAM& operator=(const VolumeRAM& that) = default;
    VolumeRAM& operator=(VolumeRAM&& that) = default;
};

class Volume;

template <>
struct representation_traits<Volume, kind::RAM> {
    using type = VolumeRAM;
};

/**
 * \ingroup datastructures
 */
template <typename T>
class VolumeRAMPrecision : public VolumeRAM {
public:
    using type = T;

    explicit VolumeRAMPrecision(size3_t dimensions = size3_t(128, 128, 128),
                                const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                                InterpolationType interpolation = InterpolationType::Linear,
                                const Wrapping3D& wrapping = wrapping3d::clampAll);
    VolumeRAMPrecision(T* data, size3_t dimensions,
                       const SwizzleMask& swizzleMask = swizzlemasks::rgba,
                       InterpolationType interpolation = InterpolationType::Linear,
                       const Wrapping3D& wrapping = wrapping3d::clampAll);
    VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs);
    VolumeRAMPrecision(NoData, const VolumeRepresentation& rhs);
    VolumeRAMPrecision<T>& operator=(const VolumeRAMPrecision<T>& that);
    virtual VolumeRAMPrecision<T>* clone() const override;
    virtual ~VolumeRAMPrecision();

    virtual const DataFormatBase* getDataFormat() const override;

    T* getDataTyped();
    const T* getDataTyped() const;

    std::span<T> getView();
    std::span<const T> getView() const;

    virtual void* getData() override;
    virtual const void* getData() const override;

    virtual void* getData(size_t) override;
    virtual const void* getData(size_t) const override;

    virtual void setData(void* data, size3_t dimensions) override;

    virtual void removeDataOwnership() override;

    virtual const size3_t& getDimensions() const override;
    virtual void setDimensions(size3_t dimensions) override;

    /**
     * \brief update the swizzle mask of the color channels when sampling the volume
     *
     * @param mask new swizzle mask
     */
    virtual void setSwizzleMask(const SwizzleMask& mask) override;
    virtual SwizzleMask getSwizzleMask() const override;

    virtual void setInterpolation(InterpolationType interpolation) override;
    virtual InterpolationType getInterpolation() const override;

    virtual void setWrapping(const Wrapping3D& wrapping) override;
    virtual Wrapping3D getWrapping() const override;

    virtual double getAsDouble(const size3_t& pos) const override;
    virtual dvec2 getAsDVec2(const size3_t& pos) const override;
    virtual dvec3 getAsDVec3(const size3_t& pos) const override;
    virtual dvec4 getAsDVec4(const size3_t& pos) const override;

    virtual void setFromDouble(const size3_t& pos, double val) override;
    virtual void setFromDVec2(const size3_t& pos, dvec2 val) override;
    virtual void setFromDVec3(const size3_t& pos, dvec3 val) override;
    virtual void setFromDVec4(const size3_t& pos, dvec4 val) override;

    virtual double getAsNormalizedDouble(const size3_t& pos) const override;
    virtual dvec2 getAsNormalizedDVec2(const size3_t& pos) const override;
    virtual dvec3 getAsNormalizedDVec3(const size3_t& pos) const override;
    virtual dvec4 getAsNormalizedDVec4(const size3_t& pos) const override;

    virtual void setFromNormalizedDouble(const size3_t& pos, double val) override;
    virtual void setFromNormalizedDVec2(const size3_t& pos, dvec2 val) override;
    virtual void setFromNormalizedDVec3(const size3_t& pos, dvec3 val) override;
    virtual void setFromNormalizedDVec4(const size3_t& pos, dvec4 val) override;

    virtual size_t getNumberOfBytes() const override;

private:
    size3_t dimensions_;
    bool ownsDataPtr_;
    std::unique_ptr<T[]> data_;
    SwizzleMask swizzleMask_;
    InterpolationType interpolation_;
    Wrapping3D wrapping_;
};

/**
 * Factory for volumes.
 * Creates an VolumeRAM with data type specified by format.
 *
 * @param dimensions of volume to create.
 * @param format of volume to create.
 * @param dataPtr optional pointer to data to be handed into the volume.
 * @param swizzleMask of volume to create.
 * @param interpolation of volume to create.
 * @param wrapping of volume to create.
 * @return nullptr if no valid format was specified.
 */
IVW_CORE_API std::shared_ptr<VolumeRAM> createVolumeRAM(
    const size3_t& dimensions, const DataFormatBase* format, void* dataPtr = nullptr,
    const SwizzleMask& swizzleMask = swizzlemasks::rgba,
    InterpolationType interpolation = InterpolationType::Linear,
    const Wrapping3D& wrapping = wrapping3d::clampAll);

template <typename T>
T VolumeRAM::posToIndex(const glm::tvec3<T, glm::defaultp>& pos,
                        const glm::tvec3<T, glm::defaultp>& dim) {
    return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
}

template <typename T>
T VolumeRAM::periodicPosToIndex(const glm::tvec3<T, glm::defaultp>& posIn,
                                const glm::tvec3<T, glm::defaultp>& dim) {
    glm::tvec3<T, glm::defaultp> pos = posIn % dim;
    return pos.x + (pos.y * dim.x) + (pos.z * dim.x * dim.y);
}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(size3_t dimensions, const SwizzleMask& swizzleMask,
                                          InterpolationType interpolation,
                                          const Wrapping3D& wrapping)
    : VolumeRAM{}
    , dimensions_{dimensions}
    , ownsDataPtr_{true}
    , data_{std::make_unique<T[]>(glm::compMul(dimensions_))}
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping} {}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(T* data, size3_t dimensions,
                                          const SwizzleMask& swizzleMask,
                                          InterpolationType interpolation,
                                          const Wrapping3D& wrapping)
    : VolumeRAM{}
    , dimensions_{dimensions}
    , ownsDataPtr_{true}
    , data_{data}
    , swizzleMask_{swizzleMask}
    , interpolation_{interpolation}
    , wrapping_{wrapping} {
    if (!data_) {
        data_ = std::make_unique<T[]>(glm::compMul(dimensions_));
    }
}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(const VolumeRAMPrecision<T>& rhs)
    : VolumeRAM{rhs}
    , dimensions_{rhs.dimensions_}
    , ownsDataPtr_{true}
    , data_{std::make_unique<T[]>(glm::compMul(dimensions_))}
    , swizzleMask_{rhs.swizzleMask_}
    , interpolation_{rhs.interpolation_}
    , wrapping_{rhs.wrapping_} {

    std::copy(rhs.getView().begin(), rhs.getView().end(), data_.get());
}

template <typename T>
VolumeRAMPrecision<T>::VolumeRAMPrecision(NoData, const VolumeRepresentation& rhs)
    : VolumeRAM{}
    , dimensions_{rhs.getDimensions()}
    , ownsDataPtr_{true}
    , data_{std::make_unique<T[]>(glm::compMul(dimensions_))}
    , swizzleMask_{rhs.getSwizzleMask()}
    , interpolation_{rhs.getInterpolation()}
    , wrapping_{rhs.getWrapping()} {}

template <typename T>
VolumeRAMPrecision<T>& VolumeRAMPrecision<T>::operator=(const VolumeRAMPrecision<T>& that) {
    if (this != &that) {
        VolumeRAM::operator=(that);
        auto dim = that.dimensions_;
        auto data = std::make_unique<T[]>(dim.x * dim.y * dim.z);
        std::copy(that.getView().begin(), that.getView().end(), data.get());
        data_.swap(data);
        std::swap(dim, dimensions_);
        ownsDataPtr_ = true;
        swizzleMask_ = that.swizzleMask_;
        interpolation_ = that.interpolation_;
        wrapping_ = that.wrapping_;
    }
    return *this;
}

template <typename T>
VolumeRAMPrecision<T>::~VolumeRAMPrecision() {
    if (!ownsDataPtr_) data_.release();
}

template <typename T>
VolumeRAMPrecision<T>* VolumeRAMPrecision<T>::clone() const {
    return new VolumeRAMPrecision<T>(*this);
}
template <typename T>
const DataFormatBase* VolumeRAMPrecision<T>::getDataFormat() const {
    return DataFormat<T>::get();
}

template <typename T>
const T* VolumeRAMPrecision<T>::getDataTyped() const {
    return data_.get();
}

template <typename T>
T* VolumeRAMPrecision<T>::getDataTyped() {
    return data_.get();
}

template <typename T>
std::span<T> VolumeRAMPrecision<T>::getView() {
    return std::span<T>{data_.get(), glm::compMul(dimensions_)};
}

template <typename T>
std::span<const T> VolumeRAMPrecision<T>::getView() const {
    return std::span<const T>{data_.get(), glm::compMul(dimensions_)};
}

template <typename T>
void* VolumeRAMPrecision<T>::getData() {
    return data_.get();
}
template <typename T>
const void* VolumeRAMPrecision<T>::getData() const {
    return const_cast<const T*>(data_.get());
}

template <typename T>
void* VolumeRAMPrecision<T>::getData(size_t pos) {
    return data_.get() + pos;
}

template <typename T>
const void* VolumeRAMPrecision<T>::getData(size_t pos) const {
    return const_cast<const T*>(data_.get()) + pos;
}

template <typename T>
void VolumeRAMPrecision<T>::setData(void* d, size3_t dimensions) {
    std::unique_ptr<T[]> data(static_cast<T*>(d));
    data_.swap(data);
    std::swap(dimensions_, dimensions);

    if (!ownsDataPtr_) data.release();
    ownsDataPtr_ = true;
}

template <typename T>
void VolumeRAMPrecision<T>::removeDataOwnership() {
    ownsDataPtr_ = false;
}

template <typename T>
const size3_t& VolumeRAMPrecision<T>::getDimensions() const {
    return dimensions_;
}

template <typename T>
size_t VolumeRAMPrecision<T>::getNumberOfBytes() const {
    return dimensions_.x * dimensions_.y * dimensions_.z * sizeof(T);
}

template <typename T>
void VolumeRAMPrecision<T>::setDimensions(size3_t dimensions) {
    if (dimensions_ != dimensions) {
        auto data = std::make_unique<T[]>(dimensions.x * dimensions.y * dimensions.z);
        data_.swap(data);
        dimensions_ = dimensions;
        if (!ownsDataPtr_) data.release();
        ownsDataPtr_ = true;
    }
}

template <typename T>
void VolumeRAMPrecision<T>::setSwizzleMask(const SwizzleMask& mask) {
    swizzleMask_ = mask;
}

template <typename T>
SwizzleMask VolumeRAMPrecision<T>::getSwizzleMask() const {
    return swizzleMask_;
}

template <typename T>
void VolumeRAMPrecision<T>::setInterpolation(InterpolationType interpolation) {
    interpolation_ = interpolation;
}

template <typename T>
InterpolationType VolumeRAMPrecision<T>::getInterpolation() const {
    return interpolation_;
}

template <typename T>
void VolumeRAMPrecision<T>::setWrapping(const Wrapping3D& wrapping) {
    wrapping_ = wrapping;
}

template <typename T>
Wrapping3D VolumeRAMPrecision<T>::getWrapping() const {
    return wrapping_;
}

template <typename T>
double VolumeRAMPrecision<T>::getAsDouble(const size3_t& pos) const {
    return util::glm_convert<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getAsDVec2(const size3_t& pos) const {
    return util::glm_convert<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getAsDVec3(const size3_t& pos) const {
    return util::glm_convert<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getAsDVec4(const size3_t& pos) const {
    return util::glm_convert<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDouble(const size3_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec2(const size3_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec3(const size3_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromDVec4(const size3_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert<T>(val);
}

template <typename T>
double VolumeRAMPrecision<T>::getAsNormalizedDouble(const size3_t& pos) const {
    return util::glm_convert_normalized<double>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec2 VolumeRAMPrecision<T>::getAsNormalizedDVec2(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec2>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec3 VolumeRAMPrecision<T>::getAsNormalizedDVec3(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec3>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
dvec4 VolumeRAMPrecision<T>::getAsNormalizedDVec4(const size3_t& pos) const {
    return util::glm_convert_normalized<dvec4>(data_[posToIndex(pos, dimensions_)]);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDouble(const size3_t& pos, double val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec2(const size3_t& pos, dvec2 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec3(const size3_t& pos, dvec3 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename T>
void VolumeRAMPrecision<T>::setFromNormalizedDVec4(const size3_t& pos, dvec4 val) {
    data_[posToIndex(pos, dimensions_)] = util::glm_convert_normalized<T>(val);
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto VolumeRAM::dispatch(Callable&& callable, Args&&... args) -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(),
        [this]<typename T>(Callable&& obj, Args... args) {
            return obj(static_cast<VolumeRAMPrecision<T>*>(this), std::forward<Args>(args)...);
        },
        std::forward<Callable>(callable), std::forward<Args>(args)...);
}

template <typename Result, template <class> class Predicate, typename Callable, typename... Args>
auto VolumeRAM::dispatch(Callable&& callable, Args&&... args) const -> Result {
    return dispatching::singleDispatch<Result, Predicate>(
        getDataFormatId(),
        [this]<typename T>(Callable&& obj, Args... args) {
            return obj(static_cast<const VolumeRAMPrecision<T>*>(this),
                       std::forward<Args>(args)...);
        },
        std::forward<Callable>(callable), std::forward<Args>(args)...);
}

}  // namespace inviwo
