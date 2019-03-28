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

#ifndef IVW_DATAGROUPREPRESENTATION_H
#define IVW_DATAGROUPREPRESENTATION_H

#include <typeindex>

namespace inviwo {

/**
 * \ingroup datastructures
 *
 *  \brief The base class for all DataGroupRepresentation objects.
 *
 *  It has reference to zero or many DataRepresentation objects, but never owns them,
 *  they are always owned by the Data object.
 *
 *  Differences between DataGroupRepresentation and DataRepresentation:
 *    - DataGroupRepresentation does not own DataRepresentation, does should never delete them.
 *    - DataGroupRepresentation becomes invalid when a child DataRepresentation is invalid.
 */

template <typename Owner>
class DataGroupRepresentation {
public:
    virtual DataGroupRepresentation* clone() const = 0;
    virtual ~DataGroupRepresentation() = default;

    virtual std::type_index getTypeIndex() const = 0;
    virtual void setOwner(Owner*) = 0;
    virtual Owner* getOwner() = 0;
    virtual const Owner* getOwner() const = 0;
    virtual bool isValid() const = 0;
    virtual void update(bool) = 0;

protected:
    DataGroupRepresentation() = default;
    DataGroupRepresentation(const DataGroupRepresentation& rhs) = default;
    DataGroupRepresentation& operator=(const DataGroupRepresentation& that) = default;
};

}  // namespace inviwo

#endif  // IVW_DATAGROUPREPRESENTATION_H
