/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/nifti/niftireader.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/volumesampler.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo;

TEST(Nifti1, avg152T1_LR) {
    // Stored in radiological convention
    const auto filename =
        filesystem::getPath(PathType::Tests, "/volumes/nifti/avg152T1_LR_nifti.nii.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(91, 109, 91), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.dataRange);
    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.valueRange);
    
    ASSERT_EQ(mat3(vec3(-182, 0, 0), vec3(0, 218, 0), vec3(0, 0, 182)), vol->getBasis());
}
TEST(Nifti1, avg152T1_RL) {
    // Stored in neurological convention. Should be displayed identically to avg152T1_LR 
    const auto filename =
        filesystem::getPath(PathType::Tests, "/volumes/nifti/avg152T1_RL_nifti.hdr.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(91, 109, 91), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.dataRange);
    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.valueRange);

    ASSERT_EQ(mat3(vec3(182, 0, 0), vec3(0, 218, 0), vec3(0, 0, 182)), vol->getBasis());
}


TEST(Nifti1, zstat1) {
    const auto filename = filesystem::getPath(PathType::Tests, "/volumes/nifti/zstat1.nii.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(64, 64, 21), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(DataFormatId::Float32, vol->getDataFormat()->getId());
}
