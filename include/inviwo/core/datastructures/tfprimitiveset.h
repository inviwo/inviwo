/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_TFPRIMITIVESET_H
#define IVW_TFPRIMITIVESET_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

enum class TFPrimitiveSetType {
    Relative,  //<! uses the normalized range [0,1] for all TF primitives
    Absolute,  //<! absolute positioning of TF primitives
};

class IVW_CORE_API TFPrimitiveSetObserver : public Observer {
public:
    virtual void onTFPrimitiveAdded(TFPrimitive* p);
    virtual void onTFPrimitiveRemoved(TFPrimitive* p);
    virtual void onTFPrimitiveChanged(const TFPrimitive* p);
};
class IVW_CORE_API TFPrimitiveSetObservable : public Observable<TFPrimitiveSetObserver> {
protected:
    void notifyTFPrimitiveAdded(TFPrimitive* p);
    void notifyTFPrimitiveRemoved(TFPrimitive* p);
    void notifyTFPrimitiveChanged(const TFPrimitive* p);
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
    TFPrimitiveSet(const std::vector<TFPrimitiveData>& values = {},
                   TFPrimitiveSetType type = TFPrimitiveSetType::Relative);
    TFPrimitiveSet(const TFPrimitiveSet& rhs);
    TFPrimitiveSet(TFPrimitiveSet&& rhs);
    TFPrimitiveSet& operator=(const TFPrimitiveSet& rhs);
    virtual ~TFPrimitiveSet() = default;

    TFPrimitiveSetType getType() const;

    size_t size() const;

    TFPrimitive* operator[](size_t i);
    const TFPrimitive* operator[](size_t i) const;

    TFPrimitive* get(size_t i);
    const TFPrimitive* get(size_t i) const;

    std::vector<TFPrimitiveData> get() const;
    std::vector<TFPrimitiveData> getSorted() const;

    /**
     * Access TFPrimitives as pair of vectors which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vectors of TFPrimitives' position and color sorted increasingly regarding position
     */
    std::pair<std::vector<float>, std::vector<vec4>> getVectors() const;

    /**
     * Add a TFPrimitive
     *
     * @param value   TFPrimitive to be added
     * @throws RangeException if TF type is relative and the primitive position is outside [0,1]
     */
    void add(const TFPrimitive& primitive);

    /**
     * Add a TFPrimitive at pos.x where pos.y is used as alpha and the color is
     * interpolated from existing TFPrimitives before and after the given position
     *
     * @param pos     pos.x refers to the position of the TFPrimitive, pos.y will be mapped
     *                to alpha
     * @throws RangeException if TF type is relative and pos.x is outside [0,1]
     */
    void add(const vec2& pos);

    /**
     * Add a TFPrimitive
     *
     * @param value   TFPrimitive to be added
     * @throws RangeException if TF type is relative and position of point is outside [0,1]
     */
    void add(const TFPrimitiveData& data);

    /**
     * Add multiple TFPrimitives
     *
     * @param values  TFPrimitives to be added
     * @throws RangeException if TF type is relative and any of the given points is outside [0,1]
     */
    void add(const std::vector<TFPrimitiveData>& primitives);

    void remove(TFPrimitive* primitive);

    void clear();

    virtual void onTFPrimitiveChange(const TFPrimitive* p);

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    /**
     * gets called every time when a primitive is added, removed, or changed before notifying
     * the observers. Can be used to invalidate the internal state of derived classes.
     */
    virtual void invalidate() {}

    friend bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);

protected:
    void add(std::unique_ptr<TFPrimitive> primitive);
    void remove(std::vector<std::unique_ptr<TFPrimitive>>::iterator it);
    void sort();

    /**
     * Interpolate the color between TFPrimitives at position v and return the respective color and
     * opacity (rgba). The range of all TFPrimitives is [0,1] when TF type is relative
     *
     * @param v   sampling position, if TF type is relative and v is outside the range [0,1] it is
     *            clamped to [0,1]
     * @return color and opacity at position v
     */
    vec4 interpolateColor(float v) const;

    void setSerializationKey(const std::string& key, const std::string& itemKey);

    std::vector<std::unique_ptr<TFPrimitive>> values_;
    std::vector<TFPrimitive*> sorted_;

private:
    const TFPrimitiveSetType type_;
    std::string serializationKey_ = "TFPrimitives";
    std::string serializationItemKey_ = "TFPrimitive";
};

inline TFPrimitiveSetType TFPrimitiveSet::getType() const { return type_; }

bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);
bool operator!=(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);

}  // namespace inviwo

#endif  // IVW_TFPRIMITIVESET_H
