/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/util/imagecache.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

ImageCache::ImageCache(std::shared_ptr<const Image> master) : valid_(true), master_(master) {}

void ImageCache::setMaster(std::shared_ptr<const Image> master) {
    master_ = master;
    valid_ = false;
}

std::shared_ptr<const Image> ImageCache::getImage(const size2_t dimensions) const {
    if (!master_) return std::shared_ptr<const Image>();

    if (master_->getDimensions() == dimensions) return master_;

    if (!valid_) {
        // Resize all map data once
        for (auto& elem : cache_) {
            master_->copyRepresentationsTo(elem.second.get());
        }
        valid_ = true;
    }

    // look for size in cache_
    auto it = cache_.find(dimensions);
    if ( it != cache_.end()) {
        return it->second;
    } else {
        auto newImage = std::shared_ptr<Image>(master_->clone());
        newImage->setDimensions(dimensions);
        master_->copyRepresentationsTo(newImage.get());
        cache_[newImage->getDimensions()] = newImage;
        return newImage;
    }
}

void ImageCache::prune(const std::vector<size2_t>& dimensions) const {
    for (auto it = cache_.begin(); it != cache_.end();) {
        if (!util::contains(dimensions, it->first)) {
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
}

void ImageCache::update(std::vector<size2_t> dimensions) {
    std::vector<std::shared_ptr<Image>> unusedImages;

    for (auto it = cache_.begin(); it != cache_.end();) {
        auto dim = std::find(dimensions.begin(), dimensions.end(), it->first);
        if (dim == dimensions.end() || it->first == master_->getDimensions()) {
            unusedImages.push_back(std::move(it->second));
            it = cache_.erase(it);
        } else {
            util::erase_remove(dimensions, *dim);
            ++it;
        }
    }

    // dimensions now contains missing sizes only
    for (auto dim : dimensions) {
        if (dim == master_->getDimensions()) continue;

        if (!unusedImages.empty()) {
            auto img = unusedImages.back();
            unusedImages.pop_back();
            img->setDimensions(dim);
            cache_[dim] = img;
            valid_ = false;
        } else {
            auto newImage = std::shared_ptr<Image>(master_->clone());
            newImage->setDimensions(dim);    
            cache_[newImage->getDimensions()] = newImage;
            valid_ = false;
        }
    }

}

void ImageCache::setInvalid() const {
    valid_ = false;
}

bool ImageCache::hasImage(const size2_t dimensions) {
    return cache_.find(dimensions) != cache_.end();
}

void ImageCache::addImage(std::shared_ptr<Image> image) {
    cache_[image->getDimensions()] = image;
}

std::shared_ptr<Image> ImageCache::releaseImage(const size2_t dimensions) {
    auto it = cache_.find(dimensions);
    if (it != cache_.end()) {
        auto ptr = it->second;
        cache_.erase(it);
        return ptr;
    } else {
        return std::shared_ptr<Image>();
    }
}

std::shared_ptr<Image> ImageCache::getUnusedImage(const std::vector<size2_t>& dimensions) {
    auto it = std::find_if(cache_.begin(), cache_.end(), [&dimensions](const Cache::value_type& elem) {
        return !util::contains(dimensions, elem.first);
    });

    if (it != cache_.end()) {
        auto ptr = it->second;
        cache_.erase(it);
        return ptr;
    } else {
        return std::shared_ptr<Image>();
    }
}

size_t ImageCache::size() const { return cache_.size(); }

}  // namespace
