/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/core/datastructures/image/imagetypes.h>
#include <inviwo/core/datastructures/volume/volumeconfig.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/representationtraits.h>
#include <inviwo/core/datastructures/datasequence.h>
#include <inviwo/core/datastructures/volume/volumerepresentation.h>
#include <inviwo/core/datastructures/unitsystem.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include<inviwo/core/util/glmvec.h>
namespace inviwo {

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_CORE_API alignas(16) GaussianOrbital{
public:
    
    GaussianOrbital();
    GaussianOrbital(const vec4& p, const vec3& coefs);
    GaussianOrbital(const GaussianOrbital& other);
    GaussianOrbital(GaussianOrbital&& other) noexcept;

    GaussianOrbital& operator=(GaussianOrbital& other);
    GaussianOrbital& operator=(GaussianOrbital&& other) noexcept;
    ~GaussianOrbital() = default;

    vec4 p;
    vec3 coefs;

    //static constexpr uvec3 colorCode{188, 101, 101};
    //static constexpr std::string_view classIdentifier{"org.inviwo.GaussianOrbital"};
    //static constexpr std::string_view dataName{"GaussianOrbital"};
    static const uvec3 colorCode;
    static const std::string classIdentifier;
    static const std::string dataName;

    
};

}  // namespace inviwo
