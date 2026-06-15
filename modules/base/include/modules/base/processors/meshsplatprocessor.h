#pragma once
#include <modules/base/algorithm/volume/volumesplat.h>

#include <inviwo/core/processors/poolprocessor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/base/basemoduledefine.h>

namespace inviwo {

class IVW_MODULE_BASE_API MeshSplatProcessor : public PoolProcessor {
public:
    MeshSplatProcessor();
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;

private:
    MeshInport meshInport_;
    VolumeOutport volumeOutport_;

    OptionProperty<util::SplatKernel> kernelType_;
    FloatProperty error_;

    BoolProperty perPointSize_;
    FloatProperty size_;
    BoolProperty perPointWeight_;
    FloatProperty weight_;

    CompositeProperty volume_;
    IntSize3Property volumeDims_;
    FloatMat4Property basis_;

    BoolProperty customRange_;
    DoubleMinMaxProperty dataRange;
    DoubleMinMaxProperty valueRange;

    StringProperty valueName;
    StringProperty valueUnit;
};

}  // namespace inviwo
