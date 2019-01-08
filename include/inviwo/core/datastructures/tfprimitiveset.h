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
#include <inviwo/core/properties/valuewrapper.h>
#include <inviwo/core/util/fileextension.h>

namespace inviwo {

enum class TFPrimitiveSetType {
    Relative,  //<! uses the normalized range [0,1] for all TF primitives
    Absolute,  //<! absolute positioning of TF primitives
};

class TFPrimitiveSet;

class IVW_CORE_API TFPrimitiveSetObserver : public Observer {
public:
    virtual void onTFPrimitiveAdded(TFPrimitive* p);
    virtual void onTFPrimitiveRemoved(TFPrimitive* p);
    virtual void onTFPrimitiveChanged(const TFPrimitive* p);
    virtual void onTFTypeChanged(const TFPrimitiveSet* primitiveSet);
};
class IVW_CORE_API TFPrimitiveSetObservable : public Observable<TFPrimitiveSetObserver> {
protected:
    void notifyTFPrimitiveAdded(TFPrimitive* p);
    void notifyTFPrimitiveRemoved(TFPrimitive* p);
    void notifyTFPrimitiveChanged(const TFPrimitive* p);
    void notifyTFTypeChanged(const TFPrimitiveSet* primitiveSet);
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
    using iterator = std::vector<TFPrimitive*>::iterator;
    using const_iterator = std::vector<TFPrimitive*>::const_iterator;

    TFPrimitiveSet(const std::vector<TFPrimitiveData>& values = {},
                   TFPrimitiveSetType type = TFPrimitiveSetType::Relative);
    TFPrimitiveSet(const TFPrimitiveSet& rhs);
    TFPrimitiveSet(TFPrimitiveSet&& rhs) = default;
    TFPrimitiveSet& operator=(const TFPrimitiveSet& rhs);
    virtual ~TFPrimitiveSet() = default;

    void setType(TFPrimitiveSetType type);
    TFPrimitiveSetType getType() const;

    void setTitle(const std::string& title);
    const std::string& getTitle() const;

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
    TFPrimitive* operator[](size_t i);
    const TFPrimitive* operator[](size_t i) const;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    TFPrimitive* get(size_t i);
    const TFPrimitive* get(size_t i) const;

    std::vector<TFPrimitiveData> get() const;

    // get a list of TF primitives in the order they were created
    std::vector<TFPrimitiveData> getUnsorted() const;

    /**
     * Access TFPrimitives as pair of vectors which can be used, e.g., for setting uniforms of a
     * shader.
     *
     * @return vectors of TFPrimitives' position and color sorted increasingly regarding position
     */
    std::pair<std::vector<double>, std::vector<vec4>> getVectors() const;
    std::pair<std::vector<float>, std::vector<vec4>> getVectorsf() const;

    /**
     * Add a TFPrimitive
     *
     * @param primitive   TFPrimitive to be added
     * @throws RangeException if TF type is relative and the primitive position is outside [0,1]
     */
    void add(const TFPrimitive& primitive);

    /**
     * Add a TFPrimitive
     *
     * @param pos     refers to the position of the TFPrimitive
     * @param color   color of the TFPrimitive including alpha
     * @throws RangeException if TF type is relative and pos is outside [0,1]
     */
    void add(double pos, const vec4& color);

    /**
     * Add a TFPrimitive at pos.x where pos.y is used as alpha and the color is
     * interpolated from existing TFPrimitives before and after the given position
     *
     * @param pos     pos.x refers to the position of the TFPrimitive, pos.y will be mapped
     *                to alpha
     * @throws RangeException if TF type is relative and pos.x is outside [0,1]
     */
    void add(const dvec2& pos);

    /**
     * Add a TFPrimitive
     *
     * @param data   Primitive to be added
     * @throws RangeException if TF type is relative and position of point is outside [0,1]
     */
    void add(const TFPrimitiveData& data);

    /**
     * Add multiple TFPrimitives
     *
     * @param primitives  vector of primitives to be added
     * @throws RangeException if TF type is relative and any of the given points is outside [0,1]
     */
    void add(const std::vector<TFPrimitiveData>& primitives);

    void remove(TFPrimitive* primitive);

    void clear();

    void setPosition(const std::vector<TFPrimitive*> primitives, double pos);
    void setAlpha(const std::vector<TFPrimitive*> primitives, double alpha);
    void setColor(const std::vector<TFPrimitive*> primitives, const vec3& color);

    virtual void onTFPrimitiveChange(const TFPrimitive* p);

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    /**
     * gets called every time when a primitive is added, removed, or changed before notifying
     * the observers. Can be used to invalidate the internal state of derived classes.
     */
    virtual void invalidate() {}

    virtual std::vector<FileExtension> getSupportedExtensions() const;
    virtual void save(const std::string& filename,
                      const FileExtension& ext = FileExtension()) const;
    virtual void load(const std::string& filename, const FileExtension& ext = FileExtension());

    friend bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);

protected:
    void add(std::unique_ptr<TFPrimitive> primitive);
    void remove(std::vector<std::unique_ptr<TFPrimitive>>::iterator it);
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

    void setSerializationKey(const std::string& key, const std::string& itemKey);

    std::vector<std::unique_ptr<TFPrimitive>> values_;
    std::vector<TFPrimitive*> sorted_;

private:
    ValueWrapper<TFPrimitiveSetType> type_;
    std::string serializationKey_ = "TFPrimitives";
    std::string serializationItemKey_ = "TFPrimitive";

    std::string title_ = "TFPrimitiveSet";
};

inline TFPrimitiveSetType TFPrimitiveSet::getType() const { return type_; }
inline const std::string& TFPrimitiveSet::getTitle() const { return title_; }

bool operator==(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);
bool operator!=(const TFPrimitiveSet& lhs, const TFPrimitiveSet& rhs);

}  // namespace inviwo

#endif  // IVW_TFPRIMITIVESET_H
