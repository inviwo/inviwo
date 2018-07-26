#ifndef IVW_STREAMLINES_H
#define IVW_STREAMLINES_H

#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>

#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/vectorfieldvisualization/ports/seedpointsport.h>
#include <modules/vectorfieldvisualization/properties/streamlineproperties.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <inviwo/core/util/spatialsampler.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {

class SimpleMesh;
class VolumeRAM;

class IVW_MODULE_VECTORFIELDVISUALIZATION_API StreamLinesDeprecated : public Processor {
public:
    // friend void vectorvis::convertProcessor(StreamLinesDeprecated*);
    StreamLinesDeprecated();
    virtual ~StreamLinesDeprecated();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

protected:
    DataInport<SpatialSampler<3, 3, double>> sampler_;
    SeedPoints3DInport seedPoints_;
    VolumeInport volume_;
    MeshOutport linesStripsMesh_;
    IntegralLineSetOutport lines_;

    StreamLineProperties streamLineProperties_;

    TransferFunctionProperty tf_;
    FloatProperty velocityScale_;
    StringProperty maxVelocity_;

    BoolProperty useMutliThreading_;
};

}  // namespace inviwo

#endif  // IVW_STREAMLINES_H
