/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/algorithm/optimaltransport.h>
#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/datastructures/transferfunction.h>

#include <cmath>
#include <numeric>
#include <algorithm>

namespace inviwo {

namespace {

using TF = std::vector<TFPrimitiveData>;

// Helper: compute total alpha mass of a piecewise-linear TF via trapezoidal integration.
double totalMass(std::span<const TFPrimitiveData> tf) {
    if (tf.size() < 2) return 0.0;
    double mass = 0.0;
    for (std::size_t i = 1; i < tf.size(); ++i) {
        double dx = tf[i].pos - tf[i - 1].pos;
        if (dx <= 0.0) continue;
        mass += 0.5 *
                (std::max(0.0, static_cast<double>(tf[i - 1].color.a)) +
                 std::max(0.0, static_cast<double>(tf[i].color.a))) *
                dx;
    }
    return mass;
}

// A TF that is a single spike (triangle) centered at `center` with half-width `hw`.
TF spikeTF(double center, double hw, vec4 color) {
    vec4 zero = vec4{vec3{color}, 0.0f};
    return {{center - hw, zero}, {center, color}, {center + hw, zero}};
}

}  // namespace

// -------------------------------------------------------------------
// Identity tests: interpolating a TF with itself should return itself.
// -------------------------------------------------------------------

TEST(OptimalTransport, IdentityInterpolation) {
    TF tf = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.8f}},
             {0.5, vec4{0.0f, 1.0f, 0.0f, 1.0f}},
             {1.0, vec4{0.0f, 0.0f, 1.0f, 0.5f}}};

    auto result = algorithm::optimalTransportInterpolation(tf, tf, 0.5);

    // The result should have the same total mass as the input.
    double massTF = totalMass(tf);
    double massResult = totalMass(result);
    EXPECT_NEAR(massResult, massTF, 1e-3) << "Self-interpolation should preserve total mass";
}

// -------------------------------------------------------------------
// Boundary tests: t=0 returns A, t=1 returns B.
// -------------------------------------------------------------------

TEST(OptimalTransport, BoundaryT0ReturnsA) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.0);

    // Should be identical to a.
    ASSERT_EQ(result.size(), a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_DOUBLE_EQ(result[i].pos, a[i].pos);
        EXPECT_EQ(result[i].color, a[i].color);
    }
}

TEST(OptimalTransport, BoundaryT1ReturnsB) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    auto result = algorithm::optimalTransportInterpolation(a, b, 1.0);

    ASSERT_EQ(result.size(), b.size());
    for (std::size_t i = 0; i < b.size(); ++i) {
        EXPECT_DOUBLE_EQ(result[i].pos, b[i].pos);
        EXPECT_EQ(result[i].color, b[i].color);
    }
}

// -------------------------------------------------------------------
// Mass conservation: interpolation should conserve total alpha mass
// via linear interpolation of masses.
// -------------------------------------------------------------------

TEST(OptimalTransport, MassConservationUniform) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.5f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 0.5f}}};

    double massA = totalMass(a);
    double massB = totalMass(b);

    for (double t : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        double expected = (1.0 - t) * massA + t * massB;
        double actual = totalMass(result);
        EXPECT_NEAR(actual, expected, 1e-3)
            << "Mass not conserved at t=" << t << " expected=" << expected << " actual=" << actual;
    }
}

TEST(OptimalTransport, MassConservationSpikes) {
    TF a = spikeTF(0.25, 0.1, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.75, 0.1, vec4{0.0f, 0.0f, 1.0f, 1.0f});

    double massA = totalMass(a);
    double massB = totalMass(b);

    for (double t : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        double expected = (1.0 - t) * massA + t * massB;
        double actual = totalMass(result);
        EXPECT_NEAR(actual, expected, 1e-2) << "Mass not conserved for spikes at t=" << t;
    }
}

// -------------------------------------------------------------------
// Translation test: OT should translate a spike from one location
// to another without splitting.
// -------------------------------------------------------------------

TEST(OptimalTransport, SpikeTranslation) {
    // Two identical spikes at different positions.
    TF a = spikeTF(0.2, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.8, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);

    // At t=0.5, the spike should be centered around 0.5.
    // Find position of maximum alpha.
    double maxAlpha = 0.0;
    double maxPos = 0.0;
    for (const auto& p : result) {
        if (static_cast<double>(p.color.a) > maxAlpha) {
            maxAlpha = static_cast<double>(p.color.a);
            maxPos = p.pos;
        }
    }

    EXPECT_NEAR(maxPos, 0.5, 0.05) << "Spike center should be at 0.5 for t=0.5";
}

TEST(OptimalTransport, SpikeTranslationMonotone) {
    // The center of mass should move monotonically from A to B.
    TF a = spikeTF(0.2, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.8, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});

    double prevCenter = 0.0;
    for (int step = 0; step <= 10; ++step) {
        double t = step / 10.0;
        auto result = algorithm::optimalTransportInterpolation(a, b, t);

        // Compute center of mass.
        double massWeighted = 0.0;
        double mass = 0.0;
        for (std::size_t i = 1; i < result.size(); ++i) {
            double dx = result[i].pos - result[i - 1].pos;
            if (dx <= 0.0) continue;
            double a0 = std::max(0.0, static_cast<double>(result[i - 1].color.a));
            double a1 = std::max(0.0, static_cast<double>(result[i].color.a));
            double segMass = 0.5 * (a0 + a1) * dx;
            double segCenter = 0.5 * (result[i - 1].pos + result[i].pos);
            massWeighted += segMass * segCenter;
            mass += segMass;
        }
        double center = (mass > 1e-12) ? massWeighted / mass : 0.0;

        if (step > 0) {
            EXPECT_GE(center, prevCenter - 1e-6)
                << "Center of mass should move monotonically, failed at t=" << t;
        }
        prevCenter = center;
    }
}

// -------------------------------------------------------------------
// Symmetry test: interpolating A->B at t should equal B->A at (1-t).
// -------------------------------------------------------------------

TEST(OptimalTransport, Symmetry) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {0.3, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.5, vec4{1.0f, 1.0f, 0.0f, 0.5f}},
            {1.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.5, vec4{0.0f, 0.0f, 1.0f, 0.8f}},
            {0.7, vec4{0.0f, 1.0f, 1.0f, 1.0f}},
            {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    double t = 0.3;
    auto resultAB = algorithm::optimalTransportInterpolation(a, b, t);
    auto resultBA = algorithm::optimalTransportInterpolation(b, a, 1.0 - t);

    // Total mass should be the same.
    double massAB = totalMass(resultAB);
    double massBA = totalMass(resultBA);
    EXPECT_NEAR(massAB, massBA, 1e-3) << "Symmetric interpolations should have the same total mass";
}

// -------------------------------------------------------------------
// Empty / degenerate input handling.
// -------------------------------------------------------------------

TEST(OptimalTransport, EmptyAReturnsB) {
    TF a;
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);
    ASSERT_EQ(result.size(), b.size());
}

TEST(OptimalTransport, EmptyBReturnsA) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b;

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);
    ASSERT_EQ(result.size(), a.size());
}

TEST(OptimalTransport, BothEmpty) {
    TF a;
    TF b;

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);
    EXPECT_TRUE(result.empty());
}

TEST(OptimalTransport, SinglePointTF) {
    TF a = {{0.5, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.5, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    // Single-point TFs have zero mass (need at least two points for area).
    // Should fall back to linear blend or handle gracefully.
    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);
    EXPECT_FALSE(result.empty());
}

// -------------------------------------------------------------------
// Zero alpha: if both TFs have zero alpha everywhere, result should too.
// -------------------------------------------------------------------

TEST(OptimalTransport, ZeroAlphaFallback) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);

    double mass = totalMass(result);
    EXPECT_NEAR(mass, 0.0, 1e-12) << "Zero-alpha TFs should produce zero-alpha result";
}

// -------------------------------------------------------------------
// Non-negative alpha: the result should never have negative alpha.
// -------------------------------------------------------------------

TEST(OptimalTransport, NonNegativeAlpha) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {0.2, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.4, vec4{1.0f, 1.0f, 0.0f, 0.0f}},
            {0.6, vec4{0.0f, 1.0f, 0.0f, 0.8f}},
            {1.0, vec4{0.0f, 1.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.5, vec4{0.0f, 0.0f, 1.0f, 1.0f}},
            {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    for (double t : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        for (const auto& p : result) {
            EXPECT_GE(p.color.a, 0.0f) << "Negative alpha at pos=" << p.pos << " t=" << t;
        }
    }
}

// -------------------------------------------------------------------
// Positions in [0,1]: the result positions should stay within
// the convex hull of the input domains.
// -------------------------------------------------------------------

TEST(OptimalTransport, PositionsWithinDomain) {
    TF a = {{0.1, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {0.4, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.6, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {0.9, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    for (double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        for (const auto& p : result) {
            EXPECT_GE(p.pos, 0.1 - 1e-6) << "Position below domain min at t=" << t;
            EXPECT_LE(p.pos, 0.9 + 1e-6) << "Position above domain max at t=" << t;
        }
    }
}

// -------------------------------------------------------------------
// Monotone positions: result TF positions should be non-decreasing.
// -------------------------------------------------------------------

TEST(OptimalTransport, MonotonePositions) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {0.3, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.6, vec4{0.5f, 0.5f, 0.0f, 0.5f}},
            {1.0, vec4{0.0f, 1.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.4, vec4{0.0f, 0.0f, 1.0f, 0.8f}},
            {0.7, vec4{0.0f, 1.0f, 1.0f, 1.0f}},
            {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    for (double t : {0.1, 0.25, 0.5, 0.75, 0.9}) {
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        for (std::size_t i = 1; i < result.size(); ++i) {
            EXPECT_GE(result[i].pos, result[i - 1].pos - 1e-12)
                << "Non-monotone positions at index " << i << " t=" << t;
        }
    }
}

// -------------------------------------------------------------------
// Earth Mover's Distance tests.
// -------------------------------------------------------------------

TEST(OptimalTransport, EMDIdenticalIsZero) {
    TF tf = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
             {0.5, vec4{0.0f, 1.0f, 0.0f, 0.5f}},
             {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    double d = algorithm::earthMoversDistance(tf, tf);
    EXPECT_NEAR(d, 0.0, 1e-10) << "Distance from a TF to itself should be zero";
}

TEST(OptimalTransport, EMDSymmetric) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {0.5, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.5, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    double dAB = algorithm::earthMoversDistance(a, b);
    double dBA = algorithm::earthMoversDistance(b, a);
    EXPECT_NEAR(dAB, dBA, 1e-10) << "EMD should be symmetric";
}

TEST(OptimalTransport, EMDNonNegative) {
    TF a = spikeTF(0.3, 0.1, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.7, 0.1, vec4{0.0f, 0.0f, 1.0f, 1.0f});

    double d = algorithm::earthMoversDistance(a, b);
    EXPECT_GE(d, 0.0) << "EMD should be non-negative";
}

TEST(OptimalTransport, EMDTriangleInequality) {
    TF a = spikeTF(0.2, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.5, 0.05, vec4{0.0f, 1.0f, 0.0f, 1.0f});
    TF c = spikeTF(0.8, 0.05, vec4{0.0f, 0.0f, 1.0f, 1.0f});

    double dAB = algorithm::earthMoversDistance(a, b);
    double dBC = algorithm::earthMoversDistance(b, c);
    double dAC = algorithm::earthMoversDistance(a, c);

    EXPECT_LE(dAC, dAB + dBC + 1e-6) << "EMD should satisfy triangle inequality";
}

TEST(OptimalTransport, EMDTranslatedSpikes) {
    // Two uniform-mass spikes separated by distance d should have EMD = d.
    // Spike: triangle with half-width 0.05, peak alpha 1.0.
    // Mass = 0.5 * 1.0 * 0.1 = 0.05 per spike.
    // After normalization (both have equal mass), the Wasserstein-1 distance
    // equals the separation between the centers.
    TF a = spikeTF(0.3, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});
    TF b = spikeTF(0.7, 0.05, vec4{1.0f, 0.0f, 0.0f, 1.0f});

    double d = algorithm::earthMoversDistance(a, b);
    // Expected: 0.4 (distance between centers).
    EXPECT_NEAR(d, 0.4, 0.02) << "EMD of translated identical spikes should equal separation";
}

TEST(OptimalTransport, EMDEmptyInput) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b;

    double d = algorithm::earthMoversDistance(a, b);
    EXPECT_DOUBLE_EQ(d, 0.0) << "EMD with empty input should be 0";
}

// -------------------------------------------------------------------
// Color interpolation: at t=0.5 between red and blue uniform TFs,
// color should be a blend.
// -------------------------------------------------------------------

TEST(OptimalTransport, ColorInterpolation) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f, 0.0f, 0.0f, 1.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 1.0f}}};

    auto result = algorithm::optimalTransportInterpolation(a, b, 0.5);

    // Check that result colors are approximately purple (0.5, 0, 0.5).
    for (const auto& p : result) {
        if (p.color.a > 0.1f) {
            EXPECT_NEAR(p.color.r, 0.5f, 0.1f) << "Red channel should be ~0.5 at pos=" << p.pos;
            EXPECT_NEAR(p.color.b, 0.5f, 0.1f) << "Blue channel should be ~0.5 at pos=" << p.pos;
        }
    }
}

// -------------------------------------------------------------------
// Continuity: small changes in t should produce small changes in mass
// distribution (no sudden jumps).
// -------------------------------------------------------------------

TEST(OptimalTransport, ContinuityInT) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {0.2, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.4, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {1.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.6, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.8, vec4{0.0f, 0.0f, 1.0f, 1.0f}},
            {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    const int steps = 20;
    double prevMass = totalMass(a);

    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        auto result = algorithm::optimalTransportInterpolation(a, b, t);
        double mass = totalMass(result);

        // Mass should change smoothly (bounded by total mass difference / steps + tolerance).
        double maxMassDelta = std::abs(totalMass(b) - totalMass(a)) / steps + 0.05;
        EXPECT_NEAR(mass, prevMass, maxMassDelta)
            << "Mass changed too abruptly between steps at t=" << t;
        prevMass = mass;
    }
}

// -------------------------------------------------------------------
// Unsorted input: the algorithm should handle unordered TF points.
// -------------------------------------------------------------------

TEST(OptimalTransport, UnsortedInput) {
    TF a = {{0.5, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.0, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
            {1.0, vec4{1.0f, 0.0f, 0.0f, 0.2f}}};
    TF aSorted = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                  {0.5, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
                  {1.0, vec4{1.0f, 0.0f, 0.0f, 0.2f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.8f}}, {1.0, vec4{0.0f, 0.0f, 1.0f, 0.8f}}};

    auto result1 = algorithm::optimalTransportInterpolation(a, b, 0.5);
    auto result2 = algorithm::optimalTransportInterpolation(aSorted, b, 0.5);

    // Both should produce the same total mass.
    EXPECT_NEAR(totalMass(result1), totalMass(result2), 1e-6)
        << "Unsorted and sorted input should produce equivalent results";
}

// -------------------------------------------------------------------
// Samples per segment parameter: more samples should not change mass.
// -------------------------------------------------------------------
TEST(OptimalTransport, SamplesPerSegmentMass) {
    TF a = {{0.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {0.3, vec4{1.0f, 0.0f, 0.0f, 1.0f}},
            {0.6, vec4{1.0f, 0.0f, 0.0f, 0.0f}},
            {1.0, vec4{1.0f, 0.0f, 0.0f, 0.0f}}};
    TF b = {{0.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.4, vec4{0.0f, 0.0f, 1.0f, 0.0f}},
            {0.7, vec4{0.0f, 0.0f, 1.0f, 1.0f}},
            {1.0, vec4{0.0f, 0.0f, 1.0f, 0.0f}}};

    auto result4 = algorithm::optimalTransportInterpolation(a, b, 0.5, 4);
    auto result32 = algorithm::optimalTransportInterpolation(a, b, 0.5, 32);
    auto result64 = algorithm::optimalTransportInterpolation(a, b, 0.5, 64);

    double mass4 = totalMass(result4);
    double mass32 = totalMass(result32);
    double mass64 = totalMass(result64);

    double expected = 0.5 * totalMass(a) + 0.5 * totalMass(b);

    EXPECT_NEAR(mass4, expected, 0.06) << "Mass with 4 samples/segment";
    EXPECT_NEAR(mass32, expected, 0.03) << "Mass with 32 samples/segment";
    EXPECT_NEAR(mass64, expected, 0.02) << "Mass with 64 samples/segment";
}

namespace {
struct OTConf {
    static constexpr double epsilon = 2e-2;
    static constexpr size_t segments = 64;
    static constexpr int numSamples = 100;

    static const TransferFunction tfSinglePeakLeft;
    static const TransferFunction tfSinglePeakMid;
    static const TransferFunction tfSinglePeakRight;
    static const TransferFunction tfDoublePeakLeft;
    static const TransferFunction tfDoublePeakMid;
    static const TransferFunction tfDoublePeakRight;

    static void check(const TransferFunction& interpolated, const TransferFunction& reference) {
        for (int i = 0; i < OTConf::numSamples; ++i) {
            double x = i / static_cast<double>(OTConf::numSamples - 1);
            const auto interpolatedSample = interpolated.sample(x);
            const auto refSample = reference.sample(x);
            EXPECT_NEAR(interpolatedSample.a, refSample.a, OTConf::epsilon);
            EXPECT_NEAR(interpolatedSample.r, refSample.r, OTConf::epsilon);
            EXPECT_NEAR(interpolatedSample.g, refSample.g, OTConf::epsilon);
            EXPECT_NEAR(interpolatedSample.b, refSample.b, OTConf::epsilon);
        }
    }
};

const TransferFunction OTConf::tfSinglePeakLeft{{{0.1, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                 {0.2, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                 {0.3, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

const TransferFunction OTConf::tfSinglePeakMid{{{0.3, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                {0.4, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                {0.5, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

const TransferFunction OTConf::tfSinglePeakRight{{{0.5, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                  {0.6, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                  {0.7, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

const TransferFunction OTConf::tfDoublePeakLeft{{{0.1, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                 {0.15, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                 {0.2, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                 {0.8, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                 {0.85, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                 {0.9, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

const TransferFunction OTConf::tfDoublePeakMid{{{0.15, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                {0.2, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                {0.25, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                {0.75, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                {0.80, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                {0.85, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

const TransferFunction OTConf::tfDoublePeakRight{{{0.2, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                  {0.25, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                  {0.3, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                  {0.7, vec4{0.0f, 0.0f, 0.0f, 0.0f}},
                                                  {0.75, vec4{1.0f, 0.0f, 0.0f, 0.5f}},
                                                  {0.8, vec4{0.0f, 0.0f, 0.0f, 0.0f}}}};

}  // namespace

TEST(OptimalTransport, TFInterpolate1PeekMidpoint) {

    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfSinglePeakLeft.get(), OTConf::tfSinglePeakRight.get(), 0.5, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfSinglePeakMid);
}

TEST(OptimalTransport, TFInterpolate1PeekStart) {
    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfSinglePeakLeft.get(), OTConf::tfSinglePeakRight.get(), 0.00001, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfSinglePeakLeft);
}

TEST(OptimalTransport, TFInterpolate1PeekEnd) {
    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfSinglePeakLeft.get(), OTConf::tfSinglePeakRight.get(), 0.99999, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfSinglePeakRight);
}

TEST(OptimalTransport, TFInterpolate2PeekMid) {
    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfDoublePeakLeft.get(), OTConf::tfDoublePeakRight.get(), 0.5, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfDoublePeakMid);
}

TEST(OptimalTransport, TFInterpolate2PeekStart) {
    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfDoublePeakLeft.get(), OTConf::tfDoublePeakRight.get(), 0.00001, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfDoublePeakLeft);
}

TEST(OptimalTransport, TFInterpolate2PeekEnd) {
    const auto res = algorithm::optimalTransportInterpolation(
        OTConf::tfDoublePeakLeft.get(), OTConf::tfDoublePeakRight.get(), 0.99999, OTConf::segments);
    const TransferFunction interpolated{res};
    OTConf::check(interpolated, OTConf::tfDoublePeakRight);
}

}  // namespace inviwo
