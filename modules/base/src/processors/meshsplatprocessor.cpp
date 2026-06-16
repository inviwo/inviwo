
#include <modules/base/processors/meshsplatprocessor.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <fmt/core.h>

namespace inviwo {

const ProcessorInfo MeshSplatProcessor::processorInfo_{"org.inviwo.MeshGaussianSplatProcessor",
                                                       "Mesh Gaussian Splat", "Volume Creation",
                                                       CodeState::Experimental, Tags::CPU};

const ProcessorInfo& MeshSplatProcessor::getProcessorInfo() const { return processorInfo_; }

MeshSplatProcessor::MeshSplatProcessor()
    : PoolProcessor()
    , meshInport_{"meshInport"}
    , volumeOutport_{"volumeOutport"}

    , kernelType_{"kernelType",
                  "Kernel Type",
                  {{"Gaussian", "Gaussian", util::SplatKernel::Gaussian},
                   {"Epanechnikov", "Epanechnikov", util::SplatKernel::Epanechnikov},
                   {"Triangular", "Triangular", util::SplatKernel::Triangular},
                   {"Uniform", "Uniform", util::SplatKernel::Uniform}},
                  0}
    , error_{"kernelCutoff", "Kernel Cutoff (error)",
             util::ordinalScale(0.01f).setMin(0.0f).setMax(1.0f).setInc(0.00001f)}

    , perPointSize_{"perPointSize", "Use Per-Point Sizes", false}
    , size_{"size", "Size", util::ordinalScale(1.0f).setInc(0.000001f)}

    , perPointWeight_{"perPointWeight", "Use Per-Point Weights", false}
    , weight_{"weight", "Weight", util::ordinalScale(1.0f).setInc(0.000001f)}
    , volume_{"volume", "Volume Settings"}
    , volumeDims_{"volumeDims", "Volume Dimensions",
                  util::ordinalCount(size3_t{64}, size3_t(512)).setMin(size3_t(1))}
    , basis_{"basis", "Basis", util::ordinalMatrix(mat4{1.0f})}

    , customRange_{"customRange_", "Use custom range", false}
    , dataRange{"dataRange",
                "Data range",
                0.,
                1.0,
                -DataFloat64::max(),
                DataFloat64::max(),
                0.001,
                0.0,
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Text}
    , valueRange{"valueRange",
                 "Value range",
                 0.,
                 1.0,
                 -DataFloat64::max(),
                 DataFloat64::max(),
                 0.001,
                 0.0,
                 InvalidationLevel::InvalidOutput,
                 PropertySemantics::Text}
    , valueName{"valueName", "Value name", ""}
    , valueUnit{"valueUnit", "Value unit", ""} {

    addPorts(meshInport_, volumeOutport_);
    addProperties(kernelType_, perPointSize_, size_, error_, perPointWeight_, weight_, volume_);

    volume_.addProperties(volumeDims_, basis_, customRange_, dataRange, valueRange, valueName,
                          valueUnit);
}

void MeshSplatProcessor::process() {
    if (!meshInport_.hasData()) return;
    auto mesh = meshInport_.getData();

    const util::SplatSettings settings{
        .dimensions = volumeDims_.get(),
        .modelMatrix = basis_.get(),
        .axes = mesh->axes,
        .valueAxis =
            Axis{.name = valueName.get(), .unit = units::unit_from_string(valueUnit.get())},
        .kernel = kernelType_.get(),
        .size = size_.get(),
        .weight = weight_.get(),
        .error = error_.get()};

    std::span<const vec3> positions;
    std::span<const float> radii;
    std::span<const float> weights;

    const auto* positionBuffer = mesh->getBuffer(BufferType::PositionAttrib);
    positionBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto posRep) {
        using ValueType = util::PrecisionValueType<decltype(posRep)>;
        if constexpr (std::is_same_v<ValueType, vec3>) {
            positions = posRep->getDataContainer();
        }
    });

    if (perPointSize_.get()) {
        if (const auto* radiiBuffer = mesh->getBuffer(BufferType::RadiiAttrib)) {
            radiiBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto radiiRep) {
                using ValueType = util::PrecisionValueType<decltype(radiiRep)>;
                if constexpr (std::is_same_v<ValueType, float>) {
                    radii = radiiRep->getDataContainer();
                }
            });
        }
        if (radii.empty()) {
            log::warn(
                "Per-point size enabled but no valid Radii buffer found. Falling back to global "
                "size.");
        }
    }

    if (perPointWeight_.get()) {
        if (const auto* scalarBuffer = mesh->getBuffer(BufferType::ScalarMetaAttrib)) {
            scalarBuffer->getRepresentation<BufferRAM>()->dispatch<void>([&](auto scalarRep) {
                using ValueType = util::PrecisionValueType<decltype(scalarRep)>;
                if constexpr (std::is_same_v<ValueType, float>) {
                    weights = scalarRep->getDataContainer();
                }
            });
        }
        if (weights.empty()) {
            log::warn(
                "Per-point weight enabled but no valid ScalarMeta buffer found. Falling back to "
                "global weight.");
        }
    }

    auto [collect, jobs] = util::splatJobs(positions, radii, weights, settings);
    dispatchMany(jobs, [this, collect, mesh](std::vector<vec2> results) {
        auto volume = collect(std::move(results));
        volumeOutport_.setData(volume);
        newResults();
    });
}

}  // namespace inviwo
