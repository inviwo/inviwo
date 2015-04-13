/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwoapplication.h>

//Data Structures
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <inviwo/core/datastructures/geometry/geometrydisk2ramconverter.h>

//Meta Data
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>
#include <inviwo/core/metadata/propertyeditorwidgetmetadata.h>

//Utilizes
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/settings/linksettings.h>

//Io
#include <inviwo/core/io/datvolumereader.h>
#include <inviwo/core/io/ivfvolumereader.h>
#include <inviwo/core/io/datvolumewriter.h>
#include <inviwo/core/io/ivfvolumewriter.h>
#include <inviwo/core/io/rawvolumereader.h>

//Others
#include <inviwo/core/processors/canvasprocessor.h>

//Ports
#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>

//PortInspectors
#include <inviwo/core/ports/portinspector.h>

//Properties
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/planeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/advancedmaterialproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>


#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>

namespace inviwo {

InviwoCore::InviwoCore() : InviwoModule() {
    setIdentifier("Core");
    // Register Converters
    registerRepresentationConverter(new VolumeDisk2RAMConverter());
    registerRepresentationConverter(new LayerDisk2RAMConverter());
    registerRepresentationConverter(new GeometryDisk2RAMConverter());
    // Register MetaData
    registerMetaData(new BoolMetaData());
    registerMetaData(new IntMetaData());
    registerMetaData(new FloatMetaData());
    registerMetaData(new DoubleMetaData());
    registerMetaData(new StringMetaData());
    registerMetaData(new FloatVec2MetaData());
    registerMetaData(new FloatVec3MetaData());
    registerMetaData(new FloatVec4MetaData());
    registerMetaData(new DoubleVec2MetaData());
    registerMetaData(new DoubleVec3MetaData());
    registerMetaData(new DoubleVec4MetaData());
    registerMetaData(new IntVec2MetaData());
    registerMetaData(new IntVec3MetaData());
    registerMetaData(new IntVec4MetaData());
    registerMetaData(new UIntVec2MetaData());
    registerMetaData(new UIntVec3MetaData());
    registerMetaData(new UIntVec4MetaData());
    registerMetaData(new FloatMat2MetaData());
    registerMetaData(new FloatMat3MetaData());
    registerMetaData(new FloatMat4MetaData());
    registerMetaData(new DoubleMat2MetaData());
    registerMetaData(new DoubleMat4MetaData());
    registerMetaData(new DoubleMat3MetaData());
    registerMetaData(new VectorMetaData<2,float>());
    registerMetaData(new VectorMetaData<3,float>());
    registerMetaData(new VectorMetaData<4,float>());
    registerMetaData(new VectorMetaData<2,double>());
    registerMetaData(new VectorMetaData<3,double>());
    registerMetaData(new VectorMetaData<4,double>());
    registerMetaData(new VectorMetaData<2,int>());
    registerMetaData(new VectorMetaData<3,int>());
    registerMetaData(new VectorMetaData<4,int>());
    registerMetaData(new VectorMetaData<2,unsigned int>());
    registerMetaData(new VectorMetaData<3,unsigned int>());
    registerMetaData(new VectorMetaData<4,unsigned int>());
    registerMetaData(new MatrixMetaData<2,float>());
    registerMetaData(new MatrixMetaData<3,float>());
    registerMetaData(new MatrixMetaData<4,float>());
    registerMetaData(new MatrixMetaData<2,double>());
    registerMetaData(new MatrixMetaData<3,double>());
    registerMetaData(new MatrixMetaData<4,double>());
    registerMetaData(new PositionMetaData());
    registerMetaData(new ProcessorMetaData());
    registerMetaData(new ProcessorWidgetMetaData());
    registerMetaData(new PropertyEditorWidgetMetaData());
    // Register Capabilities
    registerCapabilities(new SystemCapabilities());
    // Register Data readers
    registerDataReader(new DatVolumeReader());
    registerDataReader(new IvfVolumeReader());
    registerDataReader(new RawVolumeReader());
    // Register Data writers
    registerDataWriter(new DatVolumeWriter());
    registerDataWriter(new IvfVolumeWriter());
    // Register Settings
    registerSettings(new SystemSettings());
    registerSettings(new LinkSettings());
    // Register Ports
    registerPort(GeometryInport);
    registerPort(GeometryMultiInport);
    registerPort(GeometryOutport);
    registerPort(ImageInport);
    registerPort(ImageOutport);
    registerPort(VolumeInport);
    registerPort(VolumeOutport);
    // Register PortInspectors
    registerPortInspector("org.inviwo.ImageOutport", InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_PORTINSPECTORS, "imageportinspector.inv"));
    registerPortInspector("org.inviwo.VolumeOutport", InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_PORTINSPECTORS, "volumeportinspector.inv"));
    registerPortInspector("org.inviwo.GeometryOutport", InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_PORTINSPECTORS, "geometryportinspector.inv"));
    
    //registerProperty(EventProperty); TODO fix "default" contructor with 2 args...
    registerProperty(CompositeProperty);
    registerProperty(AdvancedMaterialProperty);
    registerProperty(BoolProperty);
    registerProperty(ButtonProperty);
    registerProperty(CameraProperty);
    registerProperty(DirectoryProperty);
    registerProperty(DoubleMat2Property);
    registerProperty(DoubleMat3Property);
    registerProperty(DoubleMat4Property);
    registerProperty(DoubleProperty);
    registerProperty(DoubleVec2Property);
    registerProperty(DoubleVec3Property);
    registerProperty(DoubleVec4Property);
    registerProperty(FileProperty);
    registerProperty(FloatMat2Property);
    registerProperty(FloatMat3Property);
    registerProperty(FloatMat4Property);
    registerProperty(FloatMinMaxProperty);
    registerProperty(FloatProperty);
    registerProperty(FloatVec2Property);
    registerProperty(FloatVec3Property);
    registerProperty(FloatVec4Property);
    registerProperty(ImageEditorProperty);
    registerProperty(IntMinMaxProperty);
    registerProperty(IntProperty);
    registerProperty(IntVec2Property);
    registerProperty(IntVec3Property);
    registerProperty(IntVec4Property);
    registerProperty(OptionPropertyDouble);
    registerProperty(OptionPropertyFloat);
    registerProperty(OptionPropertyInt);
    registerProperty(OptionPropertyString);
    registerProperty(PlaneProperty);
    registerProperty(SimpleLightingProperty);
    registerProperty(SimpleRaycastingProperty);
    registerProperty(StringProperty);
    registerProperty(TransferFunctionProperty);
    registerProperty(VolumeIndicatorProperty);


    // START OF AUTOGENERATED CODE 
    // The following code has been autogenerated using createconverters.py 
    typedef OrdinalPropertyConverter<FloatProperty, IntProperty> FloatPropertyToIntPropertyConverter;
    typedef OrdinalPropertyConverter<FloatProperty, Int64Property> FloatPropertyToInt64PropertyConverter;
    typedef OrdinalPropertyConverter<FloatProperty, DoubleProperty> FloatPropertyToDoublePropertyConverter;
    typedef OrdinalPropertyConverter<IntProperty, FloatProperty> IntPropertyToFloatPropertyConverter;
    typedef OrdinalPropertyConverter<IntProperty, Int64Property> IntPropertyToInt64PropertyConverter;
    typedef OrdinalPropertyConverter<IntProperty, DoubleProperty> IntPropertyToDoublePropertyConverter;
    typedef OrdinalPropertyConverter<Int64Property, FloatProperty> Int64PropertyToFloatPropertyConverter;
    typedef OrdinalPropertyConverter<Int64Property, IntProperty> Int64PropertyToIntPropertyConverter;
    typedef OrdinalPropertyConverter<Int64Property, DoubleProperty> Int64PropertyToDoublePropertyConverter;
    typedef OrdinalPropertyConverter<DoubleProperty, FloatProperty> DoublePropertyToFloatPropertyConverter;
    typedef OrdinalPropertyConverter<DoubleProperty, IntProperty> DoublePropertyToIntPropertyConverter;
    typedef OrdinalPropertyConverter<DoubleProperty, Int64Property> DoublePropertyToInt64PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec2Property, DoubleVec2Property> FloatVec2PropertyToDoubleVec2PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec2Property, IntVec2Property> FloatVec2PropertyToIntVec2PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec2Property, FloatVec2Property> DoubleVec2PropertyToFloatVec2PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec2Property, IntVec2Property> DoubleVec2PropertyToIntVec2PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec2Property, FloatVec2Property> IntVec2PropertyToFloatVec2PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec2Property, DoubleVec2Property> IntVec2PropertyToDoubleVec2PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec3Property, DoubleVec3Property> FloatVec3PropertyToDoubleVec3PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec3Property, IntVec3Property> FloatVec3PropertyToIntVec3PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec3Property, FloatVec3Property> DoubleVec3PropertyToFloatVec3PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec3Property, IntVec3Property> DoubleVec3PropertyToIntVec3PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec3Property, FloatVec3Property> IntVec3PropertyToFloatVec3PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec3Property, DoubleVec3Property> IntVec3PropertyToDoubleVec3PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec4Property, DoubleVec4Property> FloatVec4PropertyToDoubleVec4PropertyConverter;
    typedef OrdinalPropertyConverter<FloatVec4Property, IntVec4Property> FloatVec4PropertyToIntVec4PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec4Property, FloatVec4Property> DoubleVec4PropertyToFloatVec4PropertyConverter;
    typedef OrdinalPropertyConverter<DoubleVec4Property, IntVec4Property> DoubleVec4PropertyToIntVec4PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec4Property, FloatVec4Property> IntVec4PropertyToFloatVec4PropertyConverter;
    typedef OrdinalPropertyConverter<IntVec4Property, DoubleVec4Property> IntVec4PropertyToDoubleVec4PropertyConverter;

    registerPropertyConverter(ScalarToStringConverter<FloatProperty>);
    registerPropertyConverter(ScalarToStringConverter<IntProperty>);
    registerPropertyConverter(ScalarToStringConverter<Int64Property>);
    registerPropertyConverter(ScalarToStringConverter<DoubleProperty>);
    registerPropertyConverter(VectorToStringConverter<FloatVec2Property>);
    registerPropertyConverter(VectorToStringConverter<DoubleVec2Property>);
    registerPropertyConverter(VectorToStringConverter<IntVec2Property>);
    registerPropertyConverter(VectorToStringConverter<FloatVec3Property>);
    registerPropertyConverter(VectorToStringConverter<DoubleVec3Property>);
    registerPropertyConverter(VectorToStringConverter<IntVec3Property>);
    registerPropertyConverter(VectorToStringConverter<FloatVec4Property>);
    registerPropertyConverter(VectorToStringConverter<DoubleVec4Property>);
    registerPropertyConverter(VectorToStringConverter<IntVec4Property>);

    registerPropertyConverter(FloatPropertyToIntPropertyConverter);
    registerPropertyConverter(FloatPropertyToInt64PropertyConverter);
    registerPropertyConverter(FloatPropertyToDoublePropertyConverter);
    registerPropertyConverter(IntPropertyToFloatPropertyConverter);
    registerPropertyConverter(IntPropertyToInt64PropertyConverter);
    registerPropertyConverter(IntPropertyToDoublePropertyConverter);
    registerPropertyConverter(Int64PropertyToFloatPropertyConverter);
    registerPropertyConverter(Int64PropertyToIntPropertyConverter);
    registerPropertyConverter(Int64PropertyToDoublePropertyConverter);
    registerPropertyConverter(DoublePropertyToFloatPropertyConverter);
    registerPropertyConverter(DoublePropertyToIntPropertyConverter);
    registerPropertyConverter(DoublePropertyToInt64PropertyConverter);
    registerPropertyConverter(FloatVec2PropertyToDoubleVec2PropertyConverter);
    registerPropertyConverter(FloatVec2PropertyToIntVec2PropertyConverter);
    registerPropertyConverter(DoubleVec2PropertyToFloatVec2PropertyConverter);
    registerPropertyConverter(DoubleVec2PropertyToIntVec2PropertyConverter);
    registerPropertyConverter(IntVec2PropertyToFloatVec2PropertyConverter);
    registerPropertyConverter(IntVec2PropertyToDoubleVec2PropertyConverter);
    registerPropertyConverter(FloatVec3PropertyToDoubleVec3PropertyConverter);
    registerPropertyConverter(FloatVec3PropertyToIntVec3PropertyConverter);
    registerPropertyConverter(DoubleVec3PropertyToFloatVec3PropertyConverter);
    registerPropertyConverter(DoubleVec3PropertyToIntVec3PropertyConverter);
    registerPropertyConverter(IntVec3PropertyToFloatVec3PropertyConverter);
    registerPropertyConverter(IntVec3PropertyToDoubleVec3PropertyConverter);
    registerPropertyConverter(FloatVec4PropertyToDoubleVec4PropertyConverter);
    registerPropertyConverter(FloatVec4PropertyToIntVec4PropertyConverter);
    registerPropertyConverter(DoubleVec4PropertyToFloatVec4PropertyConverter);
    registerPropertyConverter(DoubleVec4PropertyToIntVec4PropertyConverter);
    registerPropertyConverter(IntVec4PropertyToFloatVec4PropertyConverter);
    registerPropertyConverter(IntVec4PropertyToDoubleVec4PropertyConverter);

    // END OF AUTOGENERATED CODE 

}

} // namespace
