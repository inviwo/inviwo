/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/transformiterator.h>

#include <string_view>
#include <span>

namespace inviwo {

enum class TFPrimitiveSetType {
    Relative,  //<! uses the normalized range [0,1] for all TF primitives
    Absolute,  //<! absolute positioning of TF primitives
};

class TFPrimitiveSet;

class IVW_CORE_API TFPrimitiveSetObserver : public Observer {
public:
    virtual void onTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p);
    virtual void onTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p);
    virtual void onTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p);
    virtual void onTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type);
    virtual void onTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask);
};
class IVW_CORE_API TFPrimitiveSetObservable : public Observable<TFPrimitiveSetObserver> {
protected:
    void notifyTFPrimitiveAdded(const TFPrimitiveSet& set, TFPrimitive& p);
    void notifyTFPrimitiveRemoved(const TFPrimitiveSet& set, TFPrimitive& p);
    void notifyTFPrimitiveChanged(const TFPrimitiveSet& set, const TFPrimitive& p);
    void notifyTFTypeChanged(const TFPrimitiveSet& set, TFPrimitiveSetType type);
    void notifyTFMaskChanged(const TFPrimitiveSet& set, dvec2 mask);
};

/**
 * \ingroup datastructures
 * \class TFPrimitiveSet
 * \brief data structure managing multiple TFPrimitives
 *
 * \see TFPrimitive
 */
class IVW_CORE_API TFPrimitiveSet : public Serializable,
                                    public TFPrimitiveSetObservable,
                                    public TFPrimitiveObserver {
public:
    using transform_t = TFPrimitive& (*)(TFPrimitive*);
    using const_transform_t = const TFPrimitive& (*)(const TFPrimitive*);
    using iterator =
        util::TransformIterator<transform_t, typename std::vector<TFPrimitive*>::iterator>;
    using const_iterator =
        util::TransformIterator<const_transform_t,
                                typename std::vector<TFPrimitive*>::const_iterator>;

    explicit TFPrimitiveSet(const std::vector<TFPrimitiveData>& values = {},
                            TFPrimitiveSetType type = TFPrimitiveSetType::Relative);
    TFPrimitiveSet(const TFPrimitiveSet& rhs);
    TFPrimitiveSet(TFPrimitiveSet&& rhs) noexcept = default;
    TFPrimitiveSet& operator=(const TFPrimitiveSet& rhs);
    TFPrimitiveSet& operator=(TFPrimitiveSet&& rhs) noexcept;
    virtual ~TFPrimitiveSet() = default;

    void setType(TFPrimitiveSetType type);
    TFPrimitiveSetType getType() const;

    /**
     * returns the range of the TF.  For a relative TF this will return [0,1]. In case of an
     * absolute TF the range between first and last TF primitive is returned. If there are no
     * primitives in the TF [0,1] will be returned.
     *
     * @return range of TF
     */
    dvec2 getRange() const;

    size_t size() const;

    bool empty() const;

    // functions for accessing individual TF primitives which are sorted according to their position
    TFPrimitive& operator[](size_t i);
    const TFPrimitive& operator[](size_t i) const;

    TFPrimitive& get(size_t i);
    const TFPrimitive& get(size_t i) const;

    TFPrimitive& front();
    const TFPrimitive& front() const;

    TFPrimitive& back();
    const TFPrimitive& back() const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    // get a list of TF primitives in the sorted order
    std::vector<TFPrimitiveData> get() const;

    // get a list of TF primitives in the order they were created
    std::vector<TFPrimitiveData> getUnsorted() const;

    void set(std::span<const TFPrimitiveData>);

    /**
     * Set/update the list of TFPrimitives from given range
     */
    void set(const_iterator begin, const_iterator end);

    /**
     * Access TFPrimitives as pair of vectors which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vectors of TFPrimitives' position and color sorted increasingly regarding position
     */
    std::pair<std::vector<double>, std::vector<vec4>> getVectors() const;

    /**
     * Access TFPrimitives as pair of vectors which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vectors of TFPrimitives' position and color sorted increasingly regarding position
     */
    std::pair<std::vector<float>, std::vector<vec4>> getVectorsf() const;

    /**
     * Access TFPrimitives positions as a vector which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vector of TFPrimitives' positions sorted increasingly regarding position
     */
    std::vector<double> getPositions() const;
    /**
     * Access TFPrimitives positions as a vector which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vector of TFPrimitives' positions sorted increasingly regarding position
     */
    std::vector<float> getPositionsf() const;

    /**
     * Access TFPrimitives colors as a vector which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vector of TFPrimitives' colors sorted increasingly regarding position
     */
    std::vector<vec4> getColors() const;

    /**
     * Add a TFPrimitive
     *
     * @param primitive   TFPrimitive to be added
     * @throws RangeException if TF type is relative and the primitive position is outside [0,1]
     */
    TFPrimitive& add(const TFPrimitive& primitive);

    /**
     * Add a TFPrimitive
     *
     * @param pos     refers to the position of the TFPrimitive
     * @param color   color of the TFPrimitive including alpha
     * @throws RangeException if TF type is relative and pos is outside [0,1]
     */
    TFPrimitive& add(double pos, const vec4& color);

    /**
     * Add a TFPrimitive at pos with alpha. The color is
     * interpolated from existing TFPrimitives before and after the given position
     *
     * @param pos     position of the TFPrimitive,
     * @param alpha   alpha value of the TFPrimitive
     *
     * @throws RangeException if TF type is relative and pos is outside [0,1]
     */
    TFPrimitive& add(double pos, double alpha);

    /**
     * Add a TFPrimitive at pos.x where pos.y is used as alpha and the color is
     * interpolated from existing TFPrimitives before and after the given position
     *
     * @param pos     pos.x refers to the position of the TFPrimitive, pos.y will be mapped
     *                to alpha
     * @throws RangeException if TF type is relative and pos.x is outside [0,1]
     */
    TFPrimitive& add(const dvec2& pos);

    /**
     * Add a TFPrimitive
     *
     * @param data   Primitive to be added
     * @throws RangeException if TF type is relative and position of point is outside [0,1]
     */
    TFPrimitive& add(const TFPrimitiveData& data);

    /**
     * Add multiple TFPrimitives
     *
     * @param primitives  vector of primitives to be added
     * @throws RangeException if TF type is relative and any of the given points is outside [0,1]
     */
    void add(const std::vector<TFPrimitiveData>& primitives);

    /**
     * Removes a primitive from the set
     * @returns if a primitive was found and removed
     */
    bool remove(const TFPrimitive& primitive);

    void clear();

    void setPosition(std::span<TFPrimitive*> primitives, double pos);

    virtual void onTFPrimitiveChange(const TFPrimitive& p) override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    /**
     * Interpolate the color between all neighboring pairs of TFPrimitives and write the result to
     * dataArray. The range of all TFPrimitives is [0,1] when TF type is relative
     *
     * @param data   write interpolated colors into data
     */
    virtual void interpolateAndStoreColors(std::span<vec4> data) const;

    friend IVW_CORE_API bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);
    friend IVW_CORE_API bool operator!=(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);

    bool operator==(const std::vector<TFPrimitiveData>& rhs) const {
        return std::ranges::equal(
            sorted_, rhs, std::ranges::equal_to{},
            [](const TFPrimitive* p) -> const TFPrimitiveData& { return p->getData(); });
    }

    bool contains(const TFPrimitive* primitive) const;

protected:
    void verifyPoint(double pos) const;
    void verifyPoint(const TFPrimitiveData& primitive) const;
    void verifyPoint(const TFPrimitive& primitive) const;
    TFPrimitive& add(std::unique_ptr<TFPrimitive> primitive);
    bool remove(std::vector<std::unique_ptr<TFPrimitive>>::iterator it);
    void sort();

    /**
     * Interpolate the color between TFPrimitives at position t and return the respective color and
     * opacity (rgba). The range of all TFPrimitives is [0,1] when TF type is relative
     *
     * @param t   sampling position, if TF type is relative and t is outside the range [0,1] it is
     *            clamped to [0,1]
     * @return color and opacity at position t
     */
    vec4 interpolateColor(double t) const;

    virtual std::string_view serializationKey() const;
    virtual std::string_view serializationItemKey() const;

    std::vector<std::unique_ptr<TFPrimitive>> values_;
    std::vector<TFPrimitive*> sorted_;

private:
    TFPrimitiveSetType type_;
};

inline TFPrimitiveSetType TFPrimitiveSet::getType() const { return type_; }

namespace util {

IVW_CORE_API void distributeAlphaEvenly(std::vector<TFPrimitive*> selection);
IVW_CORE_API void distributePositionEvenly(std::vector<TFPrimitive*> selection);

/**
 * Set the alphas value of selection to the average alpha value.
 */
IVW_CORE_API void alignAlphaToMean(const std::vector<TFPrimitive*>& selection);
/**
 * Set the alphas value of selection to the max alpha value.
 */
IVW_CORE_API void alignAlphaToTop(const std::vector<TFPrimitive*>& selection);
/**
 * Set the alphas value of selection to the min alpha value.
 */
IVW_CORE_API void alignAlphaToBottom(const std::vector<TFPrimitive*>& selection);

/**
 * Set the position value of selection to the average position value.
 */
IVW_CORE_API void alignPositionToMean(std::vector<TFPrimitive*> selection);
/**
 * Set the position value of selection to the min position value.
 */
IVW_CORE_API void alignPositionToLeft(std::vector<TFPrimitive*> selection);

/**
 * Set the position value of selection to the max position value.
 */
IVW_CORE_API void alignPositionToRight(std::vector<TFPrimitive*> selection);

/**
 * Interpolate the alpha values of selected primitives in between the first and the last primitive
 * based on their relative position.
 */
IVW_CORE_API void interpolateAlpha(const std::vector<TFPrimitive*>& selection);

/**
 * Flip the positions of the \p selection with respect to the respective range, i.e.
 *      p' = range.max - (p - range.min)
 * with range.min/max corresponding to the lowest/highest position in \p selection.
 *
 * @param selection   list of primitives to be flipped.
 */
IVW_CORE_API void flipPositions(const std::vector<TFPrimitive*>& selection);
}  // namespace util

}  // namespace inviwo
