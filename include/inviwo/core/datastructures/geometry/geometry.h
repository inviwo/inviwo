/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_GEOMETRY_H
#define IVW_GEOMETRY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datagroup.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/geometry/geometryrepresentation.h>

namespace inviwo {

class CameraProperty;

class IVW_CORE_API Geometry : public DataGroup, public SpatialEntity<3> {

public:
    Geometry();
    Geometry(const Geometry& rhs);
    Geometry& operator=(const Geometry& that);
    virtual Geometry* clone() const;
    virtual ~Geometry();

    virtual const SpatialCameraCoordinateTransformer<3>& getCoordinateTransformer(const CameraProperty* camera) const;
    using SpatialEntity<3>::getCoordinateTransformer;
};

} // namespace

#endif // IVW_GEOMETRY_H
