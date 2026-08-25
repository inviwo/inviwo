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
#include <modules/base/properties/datarangeproperty.h>
#include <modules/base/properties/transformlistproperty.h>

#include <modules/base/basemoduledefine.h>

namespace inviwo {

class IVW_MODULE_BASE_API MeshSplatProcessor : public PoolProcessor {
public:
    MeshSplatProcessor();
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    MeshFlatMultiInport meshInport_;
    VolumeOutport volumeOutport_;

    OptionProperty<util::SplatKernel> kernelType_;
    FloatProperty error_;

    BoolProperty perPointSize_;
    FloatProperty size_;
    BoolProperty perPointWeight_;
    FloatProperty weight_;

    CompositeProperty volume_;
    IntSize3Property volumeDims_;
    TransformListProperty basis_;

    DataRangeProperty range_;

    StringProperty valueName;
    StringProperty valueUnit;
};

}  // namespace inviwo
