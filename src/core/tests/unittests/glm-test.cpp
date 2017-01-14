/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/glm.h>

#include <limits>
#include <type_traits>

namespace inviwo {


TEST(GLMTest, Extent_ivec2) {
	EXPECT_EQ(2, util::extent<ivec2>::value);
}
TEST(GLMTest, Extent_ivec3) {
	EXPECT_EQ(3, util::extent<ivec3>::value);
}
TEST(GLMTest, Extent_ivec4) {
	EXPECT_EQ(4, util::extent<ivec4>::value);
}
TEST(GLMTest, Extent_vec2) {
	EXPECT_EQ(2, util::extent<vec2>::value);
}
TEST(GLMTest, Extent_vec3) {
	EXPECT_EQ(3, util::extent<vec3>::value);
}
TEST(GLMTest, Extent_vec4) {
	EXPECT_EQ(4, util::extent<vec4>::value);
}
TEST(GLMTest, Extent_dvec2) {
	EXPECT_EQ(2, util::extent<dvec2>::value);
}
TEST(GLMTest, Extent_dvec3) {
	EXPECT_EQ(3, util::extent<dvec3>::value);
}
TEST(GLMTest, Extent_dvec4) {
	EXPECT_EQ(4, util::extent<dvec4>::value);
}
TEST(GLMTest, Extent_bvec2) {
	EXPECT_EQ(2, util::extent<bvec2>::value);
}
TEST(GLMTest, Extent_bvec3) {
	EXPECT_EQ(3, util::extent<bvec3>::value);
}
TEST(GLMTest, Extent_bvec4) {
	EXPECT_EQ(4, util::extent<bvec4>::value);
}
TEST(GLMTest, Extent_uvec2) {
	EXPECT_EQ(2, util::extent<uvec2>::value);
}
TEST(GLMTest, Extent_uvec3) {
	EXPECT_EQ(3, util::extent<uvec3>::value);
}
TEST(GLMTest, Extent_uvec4) {
	EXPECT_EQ(4, util::extent<uvec4>::value);
}
TEST(GLMTest, Extent_mat2) {
	EXPECT_EQ(2, util::extent<mat2>::value);
}
TEST(GLMTest, Extent_mat3) {
	EXPECT_EQ(3, util::extent<mat3>::value);
}
TEST(GLMTest, Extent_mat4) {
	EXPECT_EQ(4, util::extent<mat4>::value);
}
TEST(GLMTest, Extent_dmat2) {
	EXPECT_EQ(2, util::extent<dmat2>::value);
}
TEST(GLMTest, Extent_dmat3) {
	EXPECT_EQ(3, util::extent<dmat3>::value);
}
TEST(GLMTest, Extent_dmat4) {
	EXPECT_EQ(4, util::extent<dmat4>::value);
}
TEST(GLMTest, Extent_size2_t) {
	EXPECT_EQ(2, util::extent<size2_t>::value);
}
TEST(GLMTest, Extent_size3_t) {
	EXPECT_EQ(3, util::extent<size3_t>::value);
}
TEST(GLMTest, Extent_size4_t) {
	EXPECT_EQ(4, util::extent<size4_t>::value);
}

TEST(GLMTest, FlatExtent_ivec2) {
	EXPECT_EQ(2, util::flat_extent<ivec2>::value);
}
TEST(GLMTest, FlatExtent_ivec3) {
	EXPECT_EQ(3, util::flat_extent<ivec3>::value);
}
TEST(GLMTest, FlatExtent_ivec4) {
	EXPECT_EQ(4, util::flat_extent<ivec4>::value);
}
TEST(GLMTest, FlatExtent_vec2) {
	EXPECT_EQ(2, util::flat_extent<vec2>::value);
}
TEST(GLMTest, FlatExtent_vec3) {
	EXPECT_EQ(3, util::flat_extent<vec3>::value);
}
TEST(GLMTest, FlatExtent_vec4) {
	EXPECT_EQ(4, util::flat_extent<vec4>::value);
}
TEST(GLMTest, FlatExtent_dvec2) {
	EXPECT_EQ(2, util::flat_extent<dvec2>::value);
}
TEST(GLMTest, FlatExtent_dvec3) {
	EXPECT_EQ(3, util::flat_extent<dvec3>::value);
}
TEST(GLMTest, FlatExtent_dvec4) {
	EXPECT_EQ(4, util::flat_extent<dvec4>::value);
}
TEST(GLMTest, FlatExtent_bvec2) {
	EXPECT_EQ(2, util::flat_extent<bvec2>::value);
}
TEST(GLMTest, FlatExtent_bvec3) {
	EXPECT_EQ(3, util::flat_extent<bvec3>::value);
}
TEST(GLMTest, FlatExtent_bvec4) {
	EXPECT_EQ(4, util::flat_extent<bvec4>::value);
}
TEST(GLMTest, FlatExtent_uvec2) {
	EXPECT_EQ(2, util::flat_extent<uvec2>::value);
}
TEST(GLMTest, FlatExtent_uvec3) {
	EXPECT_EQ(3, util::flat_extent<uvec3>::value);
}
TEST(GLMTest, FlatExtent_uvec4) {
	EXPECT_EQ(4, util::flat_extent<uvec4>::value);
}
TEST(GLMTest, FlatExtent_mat2) {
	EXPECT_EQ(4, util::flat_extent<mat2>::value);
}
TEST(GLMTest, FlatExtent_mat3) {
	EXPECT_EQ(9, util::flat_extent<mat3>::value);
}
TEST(GLMTest, FlatExtent_mat4) {
	EXPECT_EQ(16, util::flat_extent<mat4>::value);
}
TEST(GLMTest, FlatExtent_dmat2) {
	EXPECT_EQ(4, util::flat_extent<dmat2>::value);
}
TEST(GLMTest, FlatExtent_dmat3) {
	EXPECT_EQ(9, util::flat_extent<dmat3>::value);
}
TEST(GLMTest, FlatExtent_dmat4) {
	EXPECT_EQ(16, util::flat_extent<dmat4>::value);
}
TEST(GLMTest, FlatExtent_size2_t) {
	EXPECT_EQ(2, util::flat_extent<size2_t>::value);
}
TEST(GLMTest, FlatExtent_size3_t) {
	EXPECT_EQ(3, util::flat_extent<size3_t>::value);
}
TEST(GLMTest, FlatExtent_size4_t) {
	EXPECT_EQ(4, util::flat_extent<size4_t>::value);
}

TEST(GLMTest, Extent0_ivec2) {
	const auto val = util::extent<ivec2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_ivec3) {
	const auto val = util::extent<ivec3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_ivec4) {
	const auto val = util::extent<ivec4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_vec2) {
	const auto val = util::extent<vec2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_vec3) {
	const auto val = util::extent<vec3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_vec4) {
	const auto val = util::extent<vec4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_dvec2) {
	const auto val = util::extent<dvec2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_dvec3) {
	const auto val = util::extent<dvec3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_dvec4) {
	const auto val = util::extent<dvec4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_bvec2) {
	const auto val = util::extent<bvec2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_bvec3) {
	const auto val = util::extent<bvec3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_bvec4) {
	const auto val = util::extent<bvec4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_uvec2) {
	const auto val = util::extent<uvec2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_uvec3) {
	const auto val = util::extent<uvec3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_uvec4) {
	const auto val = util::extent<uvec4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_mat2) {
	const auto val = util::extent<mat2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_mat3) {
	const auto val = util::extent<mat3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_mat4) {
	const auto val = util::extent<mat4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_dmat2) {
	const auto val = util::extent<dmat2, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_dmat3) {
	const auto val = util::extent<dmat3, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_dmat4) {
	const auto val = util::extent<dmat4, 0>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent0_size2_t) {
	const auto val = util::extent<size2_t, 0>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent0_size3_t) {
	const auto val = util::extent<size3_t, 0>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent0_size4_t) {
	const auto val = util::extent<size4_t, 0>::value;
	EXPECT_EQ(4, val);
}

TEST(GLMTest, Extent2_mat2) {
	const auto val = util::extent<mat2, 1>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent2_mat3) {
	const auto val = util::extent<mat3, 1>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent2_mat4) {
	const auto val = util::extent<mat4, 1>::value;
	EXPECT_EQ(4, val);
}
TEST(GLMTest, Extent2_dmat2) {
	const auto val = util::extent<dmat2, 1>::value;
	EXPECT_EQ(2, val);
}
TEST(GLMTest, Extent2_dmat3) {
	const auto val = util::extent<dmat3, 1>::value;
	EXPECT_EQ(3, val);
}
TEST(GLMTest, Extent2_dmat4) {
	const auto val = util::extent<dmat4, 1>::value;
	EXPECT_EQ(4, val);
}

TEST(GLMTest, Rank_ivec2) {
	EXPECT_EQ(1, util::rank<ivec2>::value);
}
TEST(GLMTest, Rank_ivec3) {
	EXPECT_EQ(1, util::rank<ivec3>::value);
}
TEST(GLMTest, Rank_ivec4) {
	EXPECT_EQ(1, util::rank<ivec4>::value);
}
TEST(GLMTest, Rank_vec2) {
	EXPECT_EQ(1, util::rank<vec2>::value);
}
TEST(GLMTest, Rank_vec3) {
	EXPECT_EQ(1, util::rank<vec3>::value);
}
TEST(GLMTest, Rank_vec4) {
	EXPECT_EQ(1, util::rank<vec4>::value);
}
TEST(GLMTest, Rank_dvec2) {
	EXPECT_EQ(1, util::rank<dvec2>::value);
}
TEST(GLMTest, Rank_dvec3) {
	EXPECT_EQ(1, util::rank<dvec3>::value);
}
TEST(GLMTest, Rank_dvec4) {
	EXPECT_EQ(1, util::rank<dvec4>::value);
}
TEST(GLMTest, Rank_bvec2) {
	EXPECT_EQ(1, util::rank<bvec2>::value);
}
TEST(GLMTest, Rank_bvec3) {
	EXPECT_EQ(1, util::rank<bvec3>::value);
}
TEST(GLMTest, Rank_bvec4) {
	EXPECT_EQ(1, util::rank<bvec4>::value);
}
TEST(GLMTest, Rank_uvec2) {
	EXPECT_EQ(1, util::rank<uvec2>::value);
}
TEST(GLMTest, Rank_uvec3) {
	EXPECT_EQ(1, util::rank<uvec3>::value);
}
TEST(GLMTest, Rank_uvec4) {
	EXPECT_EQ(1, util::rank<uvec4>::value);
}
TEST(GLMTest, Rank_mat2) {
	EXPECT_EQ(2, util::rank<mat2>::value);
}
TEST(GLMTest, Rank_mat3) {
	EXPECT_EQ(2, util::rank<mat3>::value);
}
TEST(GLMTest, Rank_mat4) {
	EXPECT_EQ(2, util::rank<mat4>::value);
}
TEST(GLMTest, Rank_dmat2) {
	EXPECT_EQ(2, util::rank<dmat2>::value);
}
TEST(GLMTest, Rank_dmat3) {
	EXPECT_EQ(2, util::rank<dmat3>::value);
}
TEST(GLMTest, Rank_dmat4) {
	EXPECT_EQ(2, util::rank<dmat4>::value);
}
TEST(GLMTest, Rank_size2_t) {
	EXPECT_EQ(1, util::rank<size2_t>::value);
}
TEST(GLMTest, Rank_size3_t) {
	EXPECT_EQ(1, util::rank<size3_t>::value);
}
TEST(GLMTest, Rank_size4_t) {
	EXPECT_EQ(1, util::rank<size4_t>::value);
}

TEST(GLMTest, glmcomp1) {
    size4_t x{0, 1, 2, 3};
    for(size_t i = 0; i < util::extent<size4_t>::value; ++i) {
        EXPECT_EQ(i, util::glmcomp(x, i));
    }
}
TEST(GLMTest, glmcomp2) {
    dvec4 x{0.0, 1.0, 2.0, 3.0};
    for(size_t i = 0; i < util::extent<dvec4>::value; ++i) {
        EXPECT_EQ(1.0*i, util::glmcomp(x, i));
    }
}
TEST(GLMTest, glmcomp3) {
    dmat4 x{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
    for(size_t i = 0; i < util::flat_extent<dmat4>::value; ++i) {
        EXPECT_EQ(1.0*i, util::glmcomp(x, i));
    }
}
TEST(GLMTest, glmcomp4) {
    dmat4 x{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0};
    for(size_t i = 0; i < util::extent<dmat4>::value; ++i) {
        for(size_t j = 0; j < util::extent<dmat4>::value; ++j) {
            EXPECT_EQ(1.0*(i*util::extent<dmat4>::value + j), util::glmcomp(x, i, j));
        }
    }
}




}