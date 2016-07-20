/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_BRUSHINGANDLINKINGMANAGER_H
#define IVW_BRUSHINGANDLINKINGMANAGER_H

#include <inviwo/core/common/inviwo.h>
#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <modules/brushingandlinking/datastructures/indexlist.h>

namespace inviwo {
class BrushingAndLinkingInport;
class BrushingAndLinkingProcessor;
/**
 * \class BrushingAndLinkingManager
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingManager {
public:
    BrushingAndLinkingManager(BrushingAndLinkingProcessor* p);
    virtual ~BrushingAndLinkingManager();

    size_t getNumberOfSelected() const;
    size_t getNumberOfFiltered() const;

    void remove(const BrushingAndLinkingInport* src);

    bool isFiltered(size_t idx) const;
    bool isSelected(size_t idx) const;

    void setSelected(const BrushingAndLinkingInport* src,
                     const std::unordered_set<size_t>& indices);

    void setFiltered(const BrushingAndLinkingInport* src,
        const std::unordered_set<size_t>& indices);

    const std::unordered_set<size_t> &getSelectedIndices() const;
    const std::unordered_set<size_t> &getFilteredIndices() const;

private:
    IndexList selected_;
    IndexList filtered_;

    std::shared_ptr<std::function<void()>> callback1_;
    std::shared_ptr<std::function<void()>> callback2_;
};

}  // namespace

#endif  // IVW_BRUSHINGANDLINKINGMANAGER_H
