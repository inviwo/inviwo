/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

namespace inviwo {

class InviwoApplication;
class Processor;
class Property;

class CameraFactory;
class DataReaderFactory;
class DataWriterFactory;
class MeshDrawerFactory;
class MetaDataFactory;
class ProcessorFactory;
class PropertyConverterManager;
class ProcessorWidgetFactory;
class DialogFactory;
class PropertyFactory;
class PropertyWidgetFactory;
class OutportFactory;
class InportFactory;
class PortInspectorFactory;
class PortInspectorManager;
class RepresentationMetaFactory;
class RepresentationConverterMetaFactory;

class ResourceManager;

namespace util {

///@{
/**
 * Camera factory
 * @see Camera CameraFactory
 */
IVW_CORE_API CameraFactory* getCameraFactory();
IVW_CORE_API CameraFactory* getCameraFactory(InviwoApplication* app);
IVW_CORE_API CameraFactory* getCameraFactory(Processor* processor);
IVW_CORE_API CameraFactory* getCameraFactory(Property* property);
///@}

///@{
/**
 * DataReader factory
 * @see DataReader DataReaderFactory
 */
IVW_CORE_API DataReaderFactory* getDataReaderFactory();
IVW_CORE_API DataReaderFactory* getDataReaderFactory(InviwoApplication* app);
IVW_CORE_API DataReaderFactory* getDataReaderFactory(Processor* processor);
IVW_CORE_API DataReaderFactory* getDataReaderFactory(Property* property);
///@}

///@{
/**
 * DataWriter factory
 * @see DataWriter DataWriterFactory
 */
IVW_CORE_API DataWriterFactory* getDataWriterFactory();
IVW_CORE_API DataWriterFactory* getDataWriterFactory(InviwoApplication* app);
IVW_CORE_API DataWriterFactory* getDataWriterFactory(Processor* processor);
IVW_CORE_API DataWriterFactory* getDataWriterFactory(Property* property);
///@}

///@{
/**
 * Dialog factory
 * @see Dialog DialogFactory
 */
IVW_CORE_API DialogFactory* getDialogFactory();
IVW_CORE_API DialogFactory* getDialogFactory(InviwoApplication* app);
IVW_CORE_API DialogFactory* getDialogFactory(Processor* processor);
IVW_CORE_API DialogFactory* getDialogFactory(Property* property);
///@}

///@{
/**
 * MeshDrawer factory
 * @see MeshDrawer MeshDrawerFactory
 */
IVW_CORE_API MeshDrawerFactory* getMeshDrawerFactory();
IVW_CORE_API MeshDrawerFactory* getMeshDrawerFactory(InviwoApplication* app);
IVW_CORE_API MeshDrawerFactory* getMeshDrawerFactory(Processor* processor);
IVW_CORE_API MeshDrawerFactory* getMeshDrawerFactory(Property* property);
///@}

///@{
/**
 * MetaData factory
 * @see MetaData MetaDataFactory
 */
IVW_CORE_API MetaDataFactory* getMetaDataFactory();
IVW_CORE_API MetaDataFactory* getMetaDataFactory(InviwoApplication* app);
IVW_CORE_API MetaDataFactory* getMetaDataFactory(Processor* processor);
IVW_CORE_API MetaDataFactory* getMetaDataFactory(Property* property);
///@}

///@{
/**
 * Inport factory
 * @see Inport InportFactory
 */
IVW_CORE_API InportFactory* getInportFactory();
IVW_CORE_API InportFactory* getInportFactory(InviwoApplication* app);
IVW_CORE_API InportFactory* getInportFactory(Processor* processor);
IVW_CORE_API InportFactory* getInportFactory(Property* property);
///@}

///@{
/**
 * Outport factory
 * @see Outport OutportFactory
 */
IVW_CORE_API OutportFactory* getOutportFactory();
IVW_CORE_API OutportFactory* getOutportFactory(InviwoApplication* app);
IVW_CORE_API OutportFactory* getOutportFactory(Processor* processor);
IVW_CORE_API OutportFactory* getOutportFactory(Property* property);
///@}

///@{
/**
 * PortInspector factory
 * @see PortInspector PortInspectorFactory
 */
IVW_CORE_API PortInspectorFactory* getPortInspectorFactory();
IVW_CORE_API PortInspectorFactory* getPortInspectorFactory(InviwoApplication* app);
IVW_CORE_API PortInspectorFactory* getPortInspectorFactory(Processor* processor);
IVW_CORE_API PortInspectorFactory* getPortInspectorFactory(Property* property);
///@}

///@{
/**
 * Processor factory
 * @see Processor ProcessorFactory
 */
IVW_CORE_API ProcessorFactory* getProcessorFactory();
IVW_CORE_API ProcessorFactory* getProcessorFactory(InviwoApplication* app);
IVW_CORE_API ProcessorFactory* getProcessorFactory(Processor* processor);
IVW_CORE_API ProcessorFactory* getProcessorFactory(Property* property);
///@}

///@{
/**
 * ProcessorWidget factory
 * @see ProcessorWidget ProcessorWidgetFactory
 */
IVW_CORE_API ProcessorWidgetFactory* getProcessorWidgetFactory();
IVW_CORE_API ProcessorWidgetFactory* getProcessorWidgetFactory(InviwoApplication* app);
IVW_CORE_API ProcessorWidgetFactory* getProcessorWidgetFactory(Processor* processor);
IVW_CORE_API ProcessorWidgetFactory* getProcessorWidgetFactory(Property* property);
///@}

///@{
/**
 * PropertyConverterManager
 * @see PropertyConverter PropertyConverterManager
 */
IVW_CORE_API PropertyConverterManager* getPropertyConverterManager();
IVW_CORE_API PropertyConverterManager* getPropertyConverterManager(InviwoApplication* app);
IVW_CORE_API PropertyConverterManager* getPropertyConverterManager(Processor* processor);
IVW_CORE_API PropertyConverterManager* getPropertyConverterManager(Property* property);
///@}

///@{
/**
 * Property factory
 * @see Property PropertyFactory
 */
IVW_CORE_API PropertyFactory* getPropertyFactory();
IVW_CORE_API PropertyFactory* getPropertyFactory(InviwoApplication* app);
IVW_CORE_API PropertyFactory* getPropertyFactory(Processor* processor);
IVW_CORE_API PropertyFactory* getPropertyFactory(Property* property);
///@}

///@{
/**
 * PropertyWidget factory
 * @see PropertyWidget PropertyWidgetFactory
 */
IVW_CORE_API PropertyWidgetFactory* getPropertyWidgetFactory();
IVW_CORE_API PropertyWidgetFactory* getPropertyWidgetFactory(InviwoApplication* app);
IVW_CORE_API PropertyWidgetFactory* getPropertyWidgetFactory(Processor* processor);
IVW_CORE_API PropertyWidgetFactory* getPropertyWidgetFactory(Property* property);
///@}

///@{
/**
 * The Representation Meta Factory holds RepresentationFactories for various kinds of
 * representations (Volume Representation, Layer Representation, Buffer Representation, etc)
 * @see Data DataRepresentation RepresentationFactory RepresentationMetaFactory
 */
IVW_CORE_API RepresentationMetaFactory* getRepresentationMetaFactory();
IVW_CORE_API RepresentationMetaFactory* getRepresentationMetaFactory(InviwoApplication* app);
IVW_CORE_API RepresentationMetaFactory* getRepresentationMetaFactory(Processor* processor);
IVW_CORE_API RepresentationMetaFactory* getRepresentationMetaFactory(Property* property);
///@}

///@{
/**
 * The Representation Converter Meta Factory holds RepresentationConverterFactories for
 * various kinds of representations (Volume Representation, Layer Representation, Buffer
 * Representation, etc)
 * @see Data DataRepresentation RepresentationConverter RepresentationConverterFactory
 * RepresentationConverterMetaFactory
 */
IVW_CORE_API RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory();
IVW_CORE_API RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(
    InviwoApplication* app);
IVW_CORE_API RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(
    Processor* processor);
IVW_CORE_API RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(
    Property* property);
///@}

///@{
/**
 * Resource Manager
 */
IVW_CORE_API ResourceManager* getResourceManager();
IVW_CORE_API ResourceManager* getResourceManager(InviwoApplication* app);
IVW_CORE_API ResourceManager* getResourceManager(Processor* processor);
IVW_CORE_API ResourceManager* getResourceManager(Property* property);
///@}

}  // namespace util

}  // namespace inviwo
