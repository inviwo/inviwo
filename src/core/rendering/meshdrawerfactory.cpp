/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

bool MeshDrawerFactory::registerObject(MeshDrawer* drawer) {
    auto res = drawers_.insert(drawer);
    return res.second;
}

bool MeshDrawerFactory::unRegisterObject(MeshDrawer* drawer) {
    auto res = drawers_.erase(drawer);
    return res > 0;
}

std::unique_ptr<MeshDrawer> MeshDrawerFactory::create(const Mesh* geom) const {
    auto it = std::find_if(drawers_.begin(), drawers_.end(),
                           [geom](MeshDrawer* d) { return d->canDraw(geom); });

    if (it != drawers_.end()) {
        return std::unique_ptr<MeshDrawer>((*it)->create(geom));
    } else {
        return std::unique_ptr<MeshDrawer>();
    }
}

bool MeshDrawerFactory::hasKey(const Mesh* geom) const {
    auto it = std::find_if(drawers_.begin(), drawers_.end(),
                           [geom](MeshDrawer* d) { return d->canDraw(geom); });

    return (it != drawers_.end());
}

}  // namespace
