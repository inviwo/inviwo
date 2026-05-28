#pragma once
#include <modules/base/algorithm/volume/volumegaussiansplat.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <modules/base/basemoduledefine.h>

namespace inviwo {

class IVW_MODULE_BASE_API MeshGaussianSplatProcessor : public Processor {
public:
    MeshGaussianSplatProcessor();
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    MeshInport meshInport_;
    VolumeOutport volumeOutport_;
    IntSize3Property volumeDims_;
    BasisProperty basis_;
    OptionProperty<util::SplatKernel> kernelType_;
    FloatProperty defaultSigma_;
    FloatProperty kernelCutoff_;
    BoolProperty usePerPointSigma_;
};

} // namespace inviwo
