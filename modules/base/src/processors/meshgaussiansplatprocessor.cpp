#include <modules/base/processors/meshgaussiansplatprocessor.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <fmt/core.h>

namespace inviwo {

const ProcessorInfo MeshGaussianSplatProcessor::processorInfo_{
    "org.inviwo.MeshGaussianSplatProcessor", "Mesh Gaussian Splat", "Volume Creation",
    CodeState::Experimental, Tags::CPU};

const ProcessorInfo& MeshGaussianSplatProcessor::getProcessorInfo() const { return processorInfo_; }

MeshGaussianSplatProcessor::MeshGaussianSplatProcessor()
    : Processor()
    , meshInport_("meshInport")
    , volumeOutport_("volumeOutport")
    , volumeDims_("volumeDims", "Volume Dimensions", size3_t(64), size3_t(1), size3_t(512))
    , basis_("basis", "Basis")
    , kernelType_("kernelType", "Kernel Type",
                  {{"Gaussian", "Gaussian", util::SplatKernel::Gaussian},
                   {"Epanechnikov", "Epanechnikov", util::SplatKernel::Epanechnikov},
                   {"Triangular", "Triangular", util::SplatKernel::Triangular},
                   {"Uniform", "Uniform", util::SplatKernel::Uniform}},
                  0)
    , defaultSigma_("defaultSigma", "Default Sigma", 1.0f, 0.01f, 10.0f)
    , kernelCutoff_("kernelCutoff", "Kernel Cutoff (error)", 0.01f, 0.0f, 0.5f)
    , usePerPointSigma_("usePerPointSigma", "Use Per-Point Sigma", false) {
    addPort(meshInport_);
    addPort(volumeOutport_);
    addProperty(volumeDims_);
    addProperty(basis_);
    addProperty(kernelType_);
    addProperty(defaultSigma_);
    addProperty(kernelCutoff_);
    addProperty(usePerPointSigma_);
}

void MeshGaussianSplatProcessor::process() {
    if (!meshInport_.hasData()) return;
    auto mesh = meshInport_.getData();

    util::GaussianSplatSettings settings{.dimensions = volumeDims_.get(),
                                         .modelMatrix = basis_.getBasisAndOffset(),
                                         .kernel = kernelType_.get(),
                                         .defaultSigma = defaultSigma_.get(),
                                         .cutoff = kernelCutoff_.get()};

    const auto* positionBuffer = mesh->getBuffer(BufferType::PositionAttrib);
    const auto* radiiBuffer = mesh->getBuffer(BufferType::RadiiAttrib);

    std::span<const vec3> positions;
    std::span<const float> radii;

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

    auto volume = util::gaussianSplat(positions, radii, settings);
    volumeOutport_.setData(volume);
}

}  // namespace inviwo
