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

#ifndef IVW_ISOVALUECOLLECTION_H
#define IVW_ISOVALUECOLLECTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/isovalue.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/fileextension.h>

#include <ostream>

namespace inviwo {

class IVW_CORE_API IsoValueCollectionObserver : public Observer {
public:
    virtual void onIsoValueAdded(IsoValue* v);
    virtual void onIsoValueRemoved(IsoValue* v);
    virtual void onIsoValueChanged(const IsoValue* v);
};
class IVW_CORE_API IsoValueCollectionObservable : public Observable<IsoValueCollectionObserver> {
protected:
    void notifyIsoValueAdded(IsoValue* v);
    void notifyIsoValueRemoved(IsoValue* v);
    void notifyIsoValueChanged(const IsoValue* v);
};

/**
 * \ingroup datastructures
 * \class IsoValueCollection
 * \brief data structure managing multiple isovalues
 *
 * \see IsoValue
 */
class IVW_CORE_API IsoValueCollection : public Serializable,
                                        public IsoValueCollectionObservable,
                                        public IsoValueObserver {
public:
    IsoValueCollection(const std::vector<IsoValue>& values = {});
    IsoValueCollection(const IsoValueCollection& rhs);
    virtual ~IsoValueCollection() = default;

    IsoValueCollection& operator=(const IsoValueCollection& rhs);

    size_t getNumIsoValues() const;

    IsoValue* getIsoValue(size_t i);
    const IsoValue* getIsoValue(size_t i) const;

    std::vector<IsoValueData> getIsoValues() const;
    std::vector<IsoValueData> getSortedIsoValues() const;

    /**
     * Add an isovalue at pos with value color
     *
     * @param value   scalar value of isovalue in range [0,1]
     * @param color   color and opacity, i.e. rgba, of the isovalue
     * @throws RangeException if isovalue is outside [0,1]
     */
    void addIsoValue(const float value, const vec4& color);

    /**
     * Add an isovalue
     *
     * @param value   isovalue to be added
     * @throws RangeException if position of point is outside [0,1]
     */
    void addIsoValue(const IsoValue& value);

    /**
     * Add an isovalue at pos.x() where pos.y is used as alpha and the color is
     * interpolated from existing isovalues before and after the given position
     *
     * @param pos     pos.x refers to the scalar isovalue in range [0,1], pos.y will be mapped
     *                to alpha
     * @throws RangeException if pos.x is outside [0,1]
     */
    void addIsoValue(const vec2& pos);

    /**
     * Add multiple isovalues
     *
     * @param values  isovalues to be added
     *
     * @throws RangeException if any of the given points is outside [0,1]
     */
    void addIsoValues(const std::vector<IsoValue>& values);

    void removeIsoValue(IsoValue* value);

    void clearIsoValues();

    virtual void onIsoValueChange(const IsoValue* v);

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    friend bool operator==(const IsoValueCollection& lhs, const IsoValueCollection& rhs);

    void save(const std::string& filename, const FileExtension& ext = FileExtension()) const;
    void load(const std::string& filename, const FileExtension& ext = FileExtension());

protected:
    void addIsoValue(std::unique_ptr<IsoValue> value);
    void removeIsoValue(std::vector<std::unique_ptr<IsoValue>>::iterator it);
    void sort();

    /**
     * Interpolate the color between isovalues at position v and return the respective color and
     * opacity (rgba). The range of all isovalues is [0,1].
     *
     * @param v   sampling position, if v is outside the range [0,1] it is clamped to [0,1]
     * @return color and opacity at position v
     */
    vec4 interpolateColor(float v) const;

private:
    std::vector<std::unique_ptr<IsoValue>> values_;
    std::vector<IsoValue*> sorted_;
};

bool operator==(const IsoValueCollection& lhs, const IsoValueCollection& rhs);
bool operator!=(const IsoValueCollection& lhs, const IsoValueCollection& rhs);

}  // namespace inviwo

#endif  // IVW_ISOVALUECOLLECTION_H
