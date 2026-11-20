/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/formats.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/storageformat.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture3d.h>
#include <modules/opengl/volume/volumegl.h>

#include <cstdint>
#include <ranges>
#include <span>

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

namespace inviwo {

namespace {

template <typename T>
std::shared_ptr<VolumeRAMPrecision<T>> makeVolumeRAM(T start, T step) {
    auto ram = std::make_shared<VolumeRAMPrecision<T>>(size3_t{4, 4, 4});
    auto view = ram->getView();
    for (size_t i = 0; i < view.size(); ++i) {
        view[i] = static_cast<T>(start + static_cast<T>(i) * step);
    }
    return ram;
}

template <typename T>
std::span<const T> volumeView(const Volume& volume) {
    const auto* ram =
        dynamic_cast<const VolumeRAMPrecision<T>*>(volume.getRepresentation<VolumeRAM>());
    EXPECT_NE(ram, nullptr);
    return ram ? ram->getView() : std::span<const T>{};
}

}  // namespace

TEST(StorageFormat, VolumeFloat32ToFloat16) {
    const auto ram = makeVolumeRAM<float>(1.0f / 3.0f, 0.1234567f);
    const std::vector<float> ref(ram->getView().begin(), ram->getView().end());
    Volume volume{ram};
    utilgl::setStorageFormat(volume, utilgl::StorageFormat::Float16);

    const auto* gl = volume.getRepresentation<VolumeGL>();
    EXPECT_TRUE(gl->isLossy());
    EXPECT_EQ(gl->getTexture()->getInternalFormat(), static_cast<GLenum>(GL_R16F));
    EXPECT_EQ(gl->getDataFormat(), DataFormat<float>::get());
    EXPECT_EQ(volume.getDataFormat(), DataFormat<float>::get());

    // The RAM representation is still the full precision source
    EXPECT_TRUE(std::ranges::equal(volumeView<float>(volume), ref));

    // Editing the GL representation promotes it to the source, RAM keeps its format
    EXPECT_FALSE(volume.getEditableRepresentation<VolumeGL>()->isLossy());
    EXPECT_EQ(volume.getDataFormat(), DataFormat<float>::get());
    const auto values = volumeView<float>(volume);
    ASSERT_EQ(values.size(), ref.size());
    EXPECT_FALSE(std::ranges::equal(values, ref));
    for (size_t i = 0; i < ref.size(); ++i) {
        EXPECT_NEAR(values[i], ref[i], std::abs(ref[i]) * 1e-3);
    }
}

TEST(StorageFormat, VolumeFloat64ToFloat32) {
    const auto ram = makeVolumeRAM<double>(1.0 / 3.0, 0.1234567);
    const std::vector<double> ref(ram->getView().begin(), ram->getView().end());
    Volume volume{ram};

    EXPECT_THROW(volume.getRepresentation<VolumeGL>(), ConverterException);

    utilgl::setStorageFormat(volume, utilgl::StorageFormat::Float32);
    const auto* gl = volume.getRepresentation<VolumeGL>();
    EXPECT_TRUE(gl->isLossy());
    EXPECT_EQ(gl->getDataFormat(), DataFormat<double>::get());
    EXPECT_EQ(gl->getTexture()->getDataFormat(), DataFormat<float>::get());
    EXPECT_TRUE(std::ranges::equal(volumeView<double>(volume), ref));

    volume.getEditableRepresentation<VolumeGL>();
    EXPECT_EQ(volume.getDataFormat(), DataFormat<double>::get());
    const auto values = volumeView<double>(volume);
    ASSERT_EQ(values.size(), ref.size());
    for (size_t i = 0; i < ref.size(); ++i) {
        EXPECT_EQ(values[i], static_cast<double>(static_cast<float>(ref[i])));
    }
}

TEST(StorageFormat, VolumeUInt16ToUNorm8) {
    const auto ram = makeVolumeRAM<std::uint16_t>(0, 1000);
    Volume volume{ram};
    utilgl::setStorageFormat(volume, utilgl::StorageFormat::UNorm8);

    const auto* gl = volume.getRepresentation<VolumeGL>();
    EXPECT_TRUE(gl->isLossy());
    EXPECT_EQ(gl->getTexture()->getInternalFormat(), static_cast<GLenum>(GL_R8));
    EXPECT_EQ(gl->getDataFormat(), DataFormat<std::uint16_t>::get());

    volume.getEditableRepresentation<VolumeGL>();
    for (const auto v : volumeView<std::uint16_t>(volume)) {
        EXPECT_EQ(v % 257, 0) << v;
    }
}

TEST(StorageFormat, UnsupportedMappingIsLossless) {
    Volume volume{makeVolumeRAM<std::uint8_t>(0, 1)};
    utilgl::setStorageFormat(volume, utilgl::StorageFormat::Float16);
    const auto* gl = volume.getRepresentation<VolumeGL>();
    EXPECT_FALSE(gl->isLossy());
    EXPECT_EQ(gl->getTexture()->getInternalFormat(), static_cast<GLenum>(GL_R8));
}

TEST(StorageFormat, ChangingStorageFormatRecreatesGLRepresentation) {
    Volume volume{makeVolumeRAM<float>(0.0f, 0.5f)};
    EXPECT_FALSE(volume.getRepresentation<VolumeGL>()->isLossy());

    // Make the GL representation the only valid one, it should survive the change
    volume.getEditableRepresentation<VolumeGL>();
    utilgl::setStorageFormat(volume, utilgl::StorageFormat::Float16);
    const auto* gl = volume.getRepresentation<VolumeGL>();
    EXPECT_TRUE(gl->isLossy());
    EXPECT_EQ(gl->getTexture()->getInternalFormat(), static_cast<GLenum>(GL_R16F));
    EXPECT_FLOAT_EQ(volumeView<float>(volume)[3], 1.5f);

    utilgl::setStorageFormat(volume, utilgl::StorageFormat::Default);
    EXPECT_FALSE(volume.getRepresentation<VolumeGL>()->isLossy());
}

TEST(StorageFormat, LayerFloat64ToFloat32) {
    auto ram = std::make_shared<LayerRAMPrecision<dvec4>>(size2_t{8, 8});
    auto view = ram->getView();
    for (size_t i = 0; i < view.size(); ++i) {
        view[i] = dvec4{1.0 / 3.0 + static_cast<double>(i)};
    }
    const std::vector<dvec4> ref(view.begin(), view.end());
    Layer layer{ram};
    utilgl::setStorageFormat(layer, utilgl::StorageFormat::Float32);

    const auto* gl = layer.getRepresentation<LayerGL>();
    EXPECT_TRUE(gl->isLossy());
    EXPECT_EQ(gl->getDataFormat(), DataFormat<dvec4>::get());
    EXPECT_EQ(gl->getTexture()->getDataFormat(), DataFormat<vec4>::get());

    layer.getEditableRepresentation<LayerGL>();
    EXPECT_EQ(layer.getDataFormat(), DataFormat<dvec4>::get());
    const auto* res =
        dynamic_cast<const LayerRAMPrecision<dvec4>*>(layer.getRepresentation<LayerRAM>());
    ASSERT_NE(res, nullptr);
    for (size_t i = 0; i < ref.size(); ++i) {
        EXPECT_EQ(res->getView()[i], dvec4{vec4{ref[i]}});
    }
}

}  // namespace inviwo
