/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <modules/nifti/niftimodule.h>
#include <modules/nifti/niftireader.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/volumesampler.h>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

using namespace inviwo;

// Information about test data can be found at
// https://nifti.nimh.nih.gov/nifti-1/data

TEST(Nifti1, avg152T1_LR) {
    // Stored in radiological convention
    // https://nipy.org/nibabel/neuro_radio_conventions.html

    // clang-format off
    /*
        header file 'avg152T1_LR_nifti.nii', num_fields = 43, fields:

        all fields:
          name                offset  nvals  values
          ------------------- ------  -----  ------
          sizeof_hdr             0      1    348 
          data_type              4     10    
          db_name               14     18    
          extents               32      1    0 
          session_error         36      1    0 
          regular               38      1    r
          dim_info              39      1    0 
          dim                   40      8    3 91 109 91 1 1 1 1 
          intent_p1             56      1    0.000000 
          intent_p2             60      1    0.000000 
          intent_p3             64      1    0.000000 
          intent_code           68      1    0 
          datatype              70      1    2 
          bitpix                72      1    8 
          slice_start           74      1    0 
          pixdim                76      8    0.000000 2.000000 2.000000 2.000000 1.000000 1.000000 1.000000 1.000000 
          vox_offset           108      1    352.000000 
          scl_slope            112      1    0.000000 
          scl_inter            116      1    0.000000 
          slice_end            120      1    0 
          slice_code           122      1    0 
          xyzt_units           123      1    10 
          cal_max              124      1    255.000000 
          cal_min              128      1    0.000000 
          slice_duration       132      1    0.000000 
          toffset              136      1    0.000000 
          glmax                140      1    0 
          glmin                144      1    0 
          descrip              148     80    FSL3.2beta
          aux_file             228     24    none                   
          qform_code           252      1    0 
          sform_code           254      1    4 
          quatern_b            256      1    0.000000 
          quatern_c            260      1    0.000000 
          quatern_d            264      1    0.000000 
          qoffset_x            268      1    0.000000 
          qoffset_y            272      1    0.000000 
          qoffset_z            276      1    0.000000 
          srow_x               280      4    -2.000000 0.000000 0.000000 90.000000 
          srow_y               296      4    0.000000 2.000000 0.000000 -126.000000 
          srow_z               312      4    0.000000 0.000000 2.000000 -72.000000 
          intent_name          328     16    
          magic                344      4    n+1
    */
    // clang-format on

    const auto filename =
        fmt::format("{}/{}",
                    InviwoApplication::getPtr()->getModuleByType<NiftiModule>()->getPath(
                        ModulePath::TestVolumes),
                    "avg152T1_LR_nifti.nii.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(91, 109, 91), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.dataRange);
    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.valueRange);

    // The x-axis should be flipped to ensure neurological convention
    ASSERT_EQ(mat3(vec3(182, 0, 0), vec3(0, 218, 0), vec3(0, 0, 182)), vol->getBasis());
}
TEST(Nifti1, avg152T1_RL) {
    // Stored in neurological convention. Should be displayed identically to avg152T1_LR
    // clang-format off
    /*
        header file 'avg152T1_RL_nifti.nii', num_fields = 43, fields:

        all fields:
          name                offset  nvals  values
          ------------------- ------  -----  ------
          sizeof_hdr             0      1    348 
          data_type              4     10    
          db_name               14     18    
          extents               32      1    0 
          session_error         36      1    0 
          regular               38      1    r
          dim_info              39      1    0 
          dim                   40      8    3 91 109 91 1 1 1 1 
          intent_p1             56      1    0.000000 
          intent_p2             60      1    0.000000 
          intent_p3             64      1    0.000000 
          intent_code           68      1    0 
          datatype              70      1    2 
          bitpix                72      1    8 
          slice_start           74      1    0 
          pixdim                76      8    0.000000 2.000000 2.000000 2.000000 1.000000 1.000000 1.000000 1.000000 
          vox_offset           108      1    352.000000 
          scl_slope            112      1    0.000000 
          scl_inter            116      1    0.000000 
          slice_end            120      1    0 
          slice_code           122      1    0 
          xyzt_units           123      1    10 
          cal_max              124      1    255.000000 
          cal_min              128      1    0.000000 
          slice_duration       132      1    0.000000 
          toffset              136      1    0.000000 
          glmax                140      1    0 
          glmin                144      1    0 
          descrip              148     80    FSL3.2beta
          aux_file             228     24    none                   
          qform_code           252      1    0 
          sform_code           254      1    4 
          quatern_b            256      1    0.000000 
          quatern_c            260      1    0.000000 
          quatern_d            264      1    0.000000 
          qoffset_x            268      1    0.000000 
          qoffset_y            272      1    0.000000 
          qoffset_z            276      1    0.000000 
          srow_x               280      4    2.000000 0.000000 0.000000 -90.000000 
          srow_y               296      4    0.000000 2.000000 0.000000 -126.000000 
          srow_z               312      4    0.000000 0.000000 2.000000 -72.000000 
          intent_name          328     16    
          magic                344      4    n+1   
    */
    // clang-format on
    const auto filename =
        fmt::format("{}/{}",
                    InviwoApplication::getPtr()->getModuleByType<NiftiModule>()->getPath(
                        ModulePath::TestVolumes),
                    "avg152T1_RL_nifti.hdr.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(91, 109, 91), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.dataRange);
    ASSERT_EQ(dvec2(0, 255), vol->dataMap_.valueRange);

    ASSERT_EQ(mat3(vec3(182, 0, 0), vec3(0, 218, 0), vec3(0, 0, 182)), vol->getBasis());
}

TEST(Nifti1, zstat1) {
    // Info about zstat1.nii:
    // clang-format off
    /*

        header file 'zstat1', num_fields = 43, fields:

        all fields:
          name                offset  nvals  values
          ------------------- ------  -----  ------
          sizeof_hdr             0      1    348 
          data_type              4     10    
          db_name               14     18    
          extents               32      1    0 
          session_error         36      1    0 
          regular               38      1    r
          dim_info              39      1    0 
          dim                   40      8    3 64 64 21 1 1 1 1 
          intent_p1             56      1    0.000000 
          intent_p2             60      1    0.000000 
          intent_p3             64      1    0.000000 
          intent_code           68      1    5 
          datatype              70      1    16 
          bitpix                72      1    32 
          slice_start           74      1    0 
          pixdim                76      8    -1.000000 4.000000 4.000000 6.000000 1.000000 1.000000 1.000000 1.000000 
          vox_offset           108      1    352.000000 
          scl_slope            112      1    0.000000 
          scl_inter            116      1    0.000000 
          slice_end            120      1    0 
          slice_code           122      1    0 
          xyzt_units           123      1    10 
          cal_max              124      1    25500.000000 
          cal_min              128      1    3.000000 
          slice_duration       132      1    0.000000 
          toffset              136      1    0.000000 
          glmax                140      1    0 
          glmin                144      1    0 
          descrip              148     80    FSL3.2beta
          aux_file             228     24    
          qform_code           252      1    1 
          sform_code           254      1    0 
          quatern_b            256      1    0.000000 
          quatern_c            260      1    1.000000 
          quatern_d            264      1    0.000000 
          qoffset_x            268      1    0.000000 
          qoffset_y            272      1    0.000000 
          qoffset_z            276      1    0.000000 
          srow_x               280      4    0.000000 0.000000 0.000000 0.000000 
          srow_y               296      4    0.000000 0.000000 0.000000 0.000000 
          srow_z               312      4    0.000000 0.000000 0.000000 0.000000 
          intent_name          328     16    
          magic                344      4    n+1

    */
    // clang-format on
    const auto filename =
        fmt::format("{}/{}",
                    InviwoApplication::getPtr()->getModuleByType<NiftiModule>()->getPath(
                        ModulePath::TestVolumes),
                    "zstat1.nii.gz");
    NiftiReader reader;
    auto vol = reader.readData(filename)->front();
    ASSERT_EQ(size3_t(64, 64, 21), vol->getDimensions()) << "Dimension mismatch";

    ASSERT_EQ(DataFormatId::Float32, vol->getDataFormat()->getId());
}
