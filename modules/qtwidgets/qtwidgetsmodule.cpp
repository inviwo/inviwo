/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmodule.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/qtwidgets/properties/anglepropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolpropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolcompositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/colorpropertywidgetqt.h>
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/eventpropertywidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/properties/fontsizepropertywidgetqt.h>
#include <modules/qtwidgets/properties/isotfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>
#include <modules/qtwidgets/properties/listpropertywidgetqt.h>
#include <modules/qtwidgets/properties/lightpropertywidgetqt.h>
#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalminmaxtextpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>
#include <modules/qtwidgets/properties/tfprimitivesetwidgetqt.h>
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>

#include <inviwo/core/io/rawvolumereader.h>
#include <modules/qtwidgets/rawdatareaderdialogqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/qtwidgetssettings.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

QtWidgetsModule::QtWidgetsModule(InviwoApplication* app)
    : InviwoModule(app, "QtWidgets"), tfMenuHelper_(util::make_unique<TFMenuHelper>()) {
    if (!qApp) {
        throw ModuleInitException("QApplication must be constructed before QtWidgetsModule");
    }
    registerSettings(util::make_unique<QtWidgetsSettings>());

    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>("Default");
    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>("Text");
    registerPropertyWidget<ButtonPropertyWidgetQt, ButtonProperty>("Default");
    registerPropertyWidget<BoolCompositePropertyWidgetQt, BoolCompositeProperty>("Default");

    registerPropertyWidget<ColorPropertyWidgetQt<ivec3>, IntVec3Property>("Color");
    registerPropertyWidget<ColorPropertyWidgetQt<ivec4>, IntVec4Property>("Color");
    registerPropertyWidget<ColorPropertyWidgetQt<vec3>, FloatVec3Property>("Color");
    registerPropertyWidget<ColorPropertyWidgetQt<vec4>, FloatVec4Property>("Color");
    registerPropertyWidget<ColorPropertyWidgetQt<dvec3>, DoubleVec3Property>("Color");
    registerPropertyWidget<ColorPropertyWidgetQt<dvec4>, DoubleVec4Property>("Color");
    registerPropertyWidget<CompositePropertyWidgetQt, CompositeProperty>("Default");
    registerPropertyWidget<EventPropertyWidgetQt, EventProperty>("Default");
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>("Default");
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::TextEditor);
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::ShaderEditor);
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::PythonEditor);
    registerPropertyWidget<FontSizePropertyWidgetQt, IntProperty>("Fontsize");
    registerPropertyWidget<MultiFilePropertyWidgetQt, MultiFileProperty>("Default");

    registerPropertyWidget<FloatMat2PropertyWidgetQt, FloatMat2Property>("Default");
    registerPropertyWidget<FloatMat3PropertyWidgetQt, FloatMat3Property>("Default");
    registerPropertyWidget<FloatMat4PropertyWidgetQt, FloatMat4Property>("Default");
    registerPropertyWidget<FloatMinMaxPropertyWidgetQt, FloatMinMaxProperty>("Default");
    registerPropertyWidget<FloatMinMaxTextPropertyWidgetQt, FloatMinMaxProperty>("Text");
    registerPropertyWidget<FloatPropertyWidgetQt, FloatProperty>("Default");
    registerPropertyWidget<FloatAnglePropertyWidgetQt, FloatProperty>("Angle");
    registerPropertyWidget<FloatPropertyWidgetQt, FloatProperty>("Text");
    registerPropertyWidget<FloatVec2PropertyWidgetQt, FloatVec2Property>("Default");
    registerPropertyWidget<FloatVec2PropertyWidgetQt, FloatVec2Property>("Text");
    registerPropertyWidget<FloatVec3PropertyWidgetQt, FloatVec3Property>("Default");
    registerPropertyWidget<FloatVec3PropertyWidgetQt, FloatVec3Property>("Text");
    registerPropertyWidget<FloatVec3PropertyWidgetQt, FloatVec3Property>("Spherical");
    registerPropertyWidget<FloatVec4PropertyWidgetQt, FloatVec4Property>("Default");
    registerPropertyWidget<FloatVec4PropertyWidgetQt, FloatVec4Property>("Text");

    registerPropertyWidget<FloatQuaternionPropertyWidgetQt, FloatQuaternionProperty>("Default");
    registerPropertyWidget<FloatQuaternionPropertyWidgetQt, FloatQuaternionProperty>("Text");

    registerPropertyWidget<DoubleMat2PropertyWidgetQt, DoubleMat2Property>("Default");
    registerPropertyWidget<DoubleMat3PropertyWidgetQt, DoubleMat3Property>("Default");
    registerPropertyWidget<DoubleMat4PropertyWidgetQt, DoubleMat4Property>("Default");
    registerPropertyWidget<DoubleMinMaxPropertyWidgetQt, DoubleMinMaxProperty>("Default");
    registerPropertyWidget<DoubleMinMaxTextPropertyWidgetQt, DoubleMinMaxProperty>("Text");
    registerPropertyWidget<DoublePropertyWidgetQt, DoubleProperty>("Default");
    registerPropertyWidget<DoubleAnglePropertyWidgetQt, DoubleProperty>("Angle");
    registerPropertyWidget<DoublePropertyWidgetQt, DoubleProperty>("Text");
    registerPropertyWidget<DoubleVec2PropertyWidgetQt, DoubleVec2Property>("Default");
    registerPropertyWidget<DoubleVec2PropertyWidgetQt, DoubleVec2Property>("Text");
    registerPropertyWidget<DoubleVec3PropertyWidgetQt, DoubleVec3Property>("Default");
    registerPropertyWidget<DoubleVec3PropertyWidgetQt, DoubleVec3Property>("Text");
    registerPropertyWidget<DoubleVec3PropertyWidgetQt, DoubleVec3Property>("Spherical");
    registerPropertyWidget<DoubleVec4PropertyWidgetQt, DoubleVec4Property>("Default");
    registerPropertyWidget<DoubleVec4PropertyWidgetQt, DoubleVec4Property>("Text");

    registerPropertyWidget<DoubleQuaternionPropertyWidgetQt, DoubleQuaternionProperty>("Default");
    registerPropertyWidget<DoubleQuaternionPropertyWidgetQt, DoubleQuaternionProperty>("Text");

    registerPropertyWidget<IntSizeTMinMaxPropertyWidgetQt, IntSizeTMinMaxProperty>("Default");
    registerPropertyWidget<Int64MinMaxPropertyWidgetQt, Int64MinMaxProperty>("Default");
    registerPropertyWidget<IntMinMaxPropertyWidgetQt, IntMinMaxProperty>("Default");
    registerPropertyWidget<IntMinMaxTextPropertyWidgetQt, IntMinMaxProperty>("Text");
    registerPropertyWidget<IntPropertyWidgetQt, IntProperty>("Default");
    registerPropertyWidget<IntPropertyWidgetQt, IntProperty>("Text");
    registerPropertyWidget<IntSizeTPropertyWidgetQt, IntSizeTProperty>("Default");
    registerPropertyWidget<IntSizeTPropertyWidgetQt, IntSizeTProperty>("Text");
    registerPropertyWidget<IntVec2PropertyWidgetQt, IntVec2Property>("Default");
    registerPropertyWidget<IntVec2PropertyWidgetQt, IntVec2Property>("Text");
    registerPropertyWidget<IntVec3PropertyWidgetQt, IntVec3Property>("Default");
    registerPropertyWidget<IntVec3PropertyWidgetQt, IntVec3Property>("Text");
    registerPropertyWidget<IntVec4PropertyWidgetQt, IntVec4Property>("Default");
    registerPropertyWidget<IntVec4PropertyWidgetQt, IntVec4Property>("Text");
    registerPropertyWidget<IntSize2PropertyWidgetQt, IntSize2Property>("Default");
    registerPropertyWidget<IntSize2PropertyWidgetQt, IntSize2Property>("Text");
    registerPropertyWidget<IntSize3PropertyWidgetQt, IntSize3Property>("Default");
    registerPropertyWidget<IntSize3PropertyWidgetQt, IntSize3Property>("Text");
    registerPropertyWidget<IntSize4PropertyWidgetQt, IntSize4Property>("Default");
    registerPropertyWidget<IntSize4PropertyWidgetQt, IntSize4Property>("Text");
    registerPropertyWidget<Int64PropertyWidgetQt, Int64Property>("Default");
    registerPropertyWidget<Int64PropertyWidgetQt, Int64Property>("Text");

    registerPropertyWidget<ListPropertyWidgetQt, ListProperty>("Default");

    registerPropertyWidget<LightPropertyWidgetQt, FloatVec3Property>("LightPosition");

    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyUInt>("Default");
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyInt>("Default");
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertySize_t>("Default");
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyFloat>("Default");
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyDouble>("Default");
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyString>("Default");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Default");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Password");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::TextEditor);
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::ShaderEditor);
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::PythonEditor);
    registerPropertyWidget<StringMultilinePropertyWidgetQt, StringProperty>("Multiline");
    registerPropertyWidget<IsoValuePropertyWidgetQt, IsoValueProperty>("Default");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>("Text");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>("Text (Normalized)");
    registerPropertyWidget<TFPropertyWidgetQt, TransferFunctionProperty>("Default");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>("Text");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>("Text (Normalized)");
    registerPropertyWidget<IsoTFPropertyWidgetQt, IsoTFProperty>("Default");
    registerPropertyWidget<CompositePropertyWidgetQt, IsoTFProperty>("Composite");

    registerDialog<RawDataReaderDialogQt>("RawVolumeReader");
    registerDialog<InviwoFileDialog>("FileDialog");
}

QtWidgetsModule::~QtWidgetsModule() = default;

TFHelpWindow* QtWidgetsModule::getTFHelpWindow() const { return tfMenuHelper_->getWindow(); }

void QtWidgetsModule::showTFHelpWindow() const { tfMenuHelper_->showWindow(); }

}  // namespace inviwo
