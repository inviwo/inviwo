/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#ifndef IVW_IMAGECACHE_H
#define IVW_IMAGECACHE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/stdextensions.h>
#include <unordered_map>

namespace inviwo {

class Image;

/**
 * \class ImageCache
 */
class IVW_CORE_API ImageCache {
public:
    ImageCache(std::shared_ptr<const Image> master = std::shared_ptr<const Image>());
    ~ImageCache() = default;

    ImageCache(const ImageCache&) = delete;
    ImageCache& operator=(const ImageCache& that) = delete;

    void setMaster(std::shared_ptr<const Image> master);
    std::shared_ptr<const Image> getImage(const size2_t dimensions) const;

    /**
     *    Remove all cached images except those in dimensions
     */
    void prune(const std::vector<size2_t>& dimensions) const;
    /**
     *    Make sure there is a cached version for all images sizes in dimensions
     */
    void update(std::vector<size2_t> dimensions);
    void setInvalid() const;

    bool hasImage(const size2_t dimensions);
    void addImage(std::shared_ptr<Image> image);
    std::shared_ptr<Image> releaseImage(const size2_t dimensions);
    std::shared_ptr<Image> getUnusedImage(const std::vector<size2_t>& dimensions);
    size_t size() const;

private:
    mutable bool valid_;
    std::shared_ptr<const Image> master_;  // non-owning reference.

    using Cache = std::unordered_map<glm::size2_t, std::shared_ptr<Image>>;
    mutable Cache cache_;
};

}  // namespace inviwo

#endif  // IVW_IMAGECACHE_H
