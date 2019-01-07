/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_MESHREPRESENTATION_H
#define IVW_MESHREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datagrouprepresentation.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class Mesh;

/**
 * \ingroup datastructures
 */
class IVW_CORE_API MeshRepresentation : public DataGroupRepresentation<Mesh> {
public:
    MeshRepresentation() = default;
    MeshRepresentation(const MeshRepresentation& rhs) = default;
    MeshRepresentation& operator=(const MeshRepresentation& that) = default;
    virtual MeshRepresentation* clone() const override = 0;
    virtual ~MeshRepresentation() = default;

    virtual std::type_index getTypeIndex() const override = 0;
    virtual void setOwner(Mesh*) override;
    virtual Mesh* getOwner() override;
    virtual const Mesh* getOwner() const override;
    virtual bool isValid() const override = 0;
    virtual void update(bool) override = 0;

protected:
    Mesh* owner_;
};

}  // namespace inviwo

#endif  // IVW_MESHREPRESENTATION_H
