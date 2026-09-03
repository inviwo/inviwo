#include <modules/base/algorithm/volume/volumesplat.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <gtest/gtest.h>

namespace inviwo {

TEST(SplatTest, SinglePointCenter) {
    Mesh mesh;
    mesh.addBuffer(BufferType::PositionAttrib,
                   std::make_shared<Buffer<vec3>>(std::make_shared<BufferRAMPrecision<vec3>>(
                       std::vector<vec3>{vec3(0.5f, 0.5f, 0.5f)})));
    mesh.addBuffer(BufferType::RadiiAttrib,
                   std::make_shared<Buffer<float>>(
                       std::make_shared<BufferRAMPrecision<float>>(std::vector<float>{0.5f})));
    const util::SplatSettings settings{
        .dimensions = size3_t(5),
        .modelMatrix = mat4(1.0f),
        .kernel = util::SplatKernel::Gaussian,
        .size = 0.2f,
        .error = 0.01f,
    };

    const auto* positionBuffer = mesh.getBuffer(BufferType::PositionAttrib);
    const auto* radiiBuffer = mesh.getBuffer(BufferType::RadiiAttrib);

    std::span<const vec3> positions;
    std::span<const float> radii;
    std::span<const float> weights;

    positionBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto posRep) {
        using ValueType = util::PrecisionValueType<decltype(posRep)>;
        if constexpr (std::is_same_v<ValueType, vec3>) {
            positions = posRep->getDataContainer();
        }
    });

    radiiBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto radiiRep) {
        using ValueType = util::PrecisionValueType<decltype(radiiRep)>;
        if constexpr (std::is_same_v<ValueType, float>) {
            radii = radiiRep->getDataContainer();
        }
    });

    auto volume =
        util::splat({.positions = positions, .sizes = radii, .weights = weights}, settings);
    ASSERT_EQ(volume->getDimensions(), size3_t(5));
    // Check that the center voxel is nonzero
    const auto* ram = volume->getRepresentation<VolumeRAM>();
    const auto center = ram->getAsDouble(size3_t(2, 2, 2));
    EXPECT_GT(center, 0.0);
}

}  // namespace inviwo
