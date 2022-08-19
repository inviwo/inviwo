/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <modules/animationqt/animationqtmodule.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/buttontrack.h>
#include <modules/animation/datastructures/cameratrack.h>
#include <modules/animation/datastructures/callbacktrack.h>
#include <modules/animation/datastructures/controltrack.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/animationcontroller.h>

#include <modules/animationqt/animationeditordockwidgetqt.h>
#include <modules/animationqt/demo/demonavigatordockwidgetqt.h>

#include <modules/animationqt/widgets/propertytrackwidgetqt.h>
#include <modules/animationqt/widgets/controltrackwidgetqt.h>

#include <modules/animationqt/sequenceeditor/propertysequenceeditor.h>
#include <modules/animationqt/sequenceeditor/controlsequenceeditor.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

#ifndef INVIWO_ALL_DYN_LINK
struct InitQtAnimationResources {
    // Needed for loading of resources when building statically
    // see https://wiki.qt.io/QtResources#Q_INIT_RESOURCE
    InitQtAnimationResources() { Q_INIT_RESOURCE(animation); }
    ~InitQtAnimationResources() { Q_CLEANUP_RESOURCE(animation); }
} initQtAnimationResources;
#endif

namespace inviwo {

namespace {

template <typename PropertyType, typename Keyframe,
          typename KeyframeSequence = animation::KeyframeSequenceTyped<Keyframe>>
void registerPropertyTrackHelper(animation::AnimationQtSupplier& as) {
    using namespace animation;

    using TrackType = PropertyTrack<PropertyType, Keyframe, KeyframeSequence>;

    as.registerTrackToWidgetMap(TrackType::classIdentifier(),
                                PropertyTrackWidgetQt::classIdentifier());

    as.registerTrackToSequenceEditorMap(TrackType::classIdentifier(),
                                        PropertySequenceEditor::classIdentifier());
}

template <template <typename> class Prop>
struct Reghelper {
    template <typename T>
    auto operator()(animation::AnimationQtSupplier& as) {
        using namespace animation;
        registerPropertyTrackHelper<Prop<T>, ValueKeyframe<typename Prop<T>::value_type>>(as);
    }
};

struct PropertyValueKeyframeReghelper {
    template <typename Prop>
    auto operator()(animation::AnimationQtSupplier& as) {
        using namespace animation;
        registerPropertyTrackHelper<Prop, ValueKeyframe<typename Prop::value_type>>(as);
    }
};

}  // namespace

AnimationQtModule::AnimationQtModule(InviwoApplication* app)
    : InviwoModule(app, "AnimationQt")
    , animation::AnimationQtSupplier(*this)
    , trackWidgetQtFactory_{}
    , sequenceEditorFactory_{} {

    using namespace animation;

    if (auto win = utilqt::getApplicationMainWindow()) {
        QString animationMenuName("Animation");
        QMenu* menu = nullptr;
        // Find view menu
        auto menus = win->menuBar()->findChildren<QMenu*>();
        auto viewMenuIt = std::find_if(menus.begin(), menus.end(), [](auto& m) {
            return m->title().compare(QObject::tr("&View"), Qt::CaseInsensitive) == 0;
        });

        if (viewMenuIt != menus.end()) {
            // Add to view menu if found
            menu = (*viewMenuIt);
        } else {
            // Add new menu if not found
            // To be removed when module is destroyed
            menu_ = std::make_unique<QMenu>(animationMenuName);
            win->menuBar()->addMenu(menu_.get());
            menu = menu_.get();
            // Release pointer if destroyed by Qt before module is destroyed
            QObject::connect(menu_.get(), &QObject::destroyed, [&](QObject*) { menu_.release(); });
        }

        const auto addWidgetLazy = [&](QString name, Qt::DockWidgetArea area, auto factory) {
            auto* action = menu->addAction(name);
            action->setCheckable(true);
            QObject::connect(action, &QAction::triggered, [action, win, area, menu, factory]() {
                // The first time the callback is called we will create the widget. And then replace
                // this action with the toggleViewAction from the new widget.
                util::OnScopeExit onExit([action]() { delete action; });
                auto widget = factory();
                win->addDockWidget(area, widget);
                widget->hide();
                widget->loadState();
                widget->setVisible(true);

                menu->insertAction(action, widget->toggleViewAction());
                menu->removeAction(action);
            });
        };

        addWidgetLazy("Animation Editor", Qt::BottomDockWidgetArea, [this, app, win]() {
            auto* animationModule = app->getModuleByType<AnimationModule>();
            editor_ = std::make_unique<AnimationEditorDockWidgetQt>(
                animationModule->getWorkspaceAnimations(), animationModule->getAnimationManager(),
                "Animation Editor", getTrackWidgetQtFactory(), getSequenceEditorFactory(), win);
            // Release pointer if destroyed by Qt before module is destroyed
            QObject::connect(editor_.get(), &QObject::destroyed,
                             [this](QObject*) { editor_.release(); });
            return editor_.get();
        });

        addWidgetLazy("Demo Navigator", Qt::RightDockWidgetArea, [this, app, win]() {
            auto* animationModule = app->getModuleByType<AnimationModule>();
            auto& demoController = animationModule->getDemoController();
            navigator_ =
                std::make_unique<DemoNavigatorDockWidgetQt>(demoController, "Demo Navigator", win);
            // Release pointer if destroyed by Qt before module is destroyed
            QObject::connect(navigator_.get(), &QObject::destroyed,
                             [this](QObject*) { navigator_.release(); });
            return navigator_.get();
        });
    }

    // register widgets
    registerTrackWidgetQt<PropertyTrackWidgetQt>();
    registerTrackWidgetQt<ControlTrackWidgetQt>();

    registerSequenceEditor<PropertySequenceEditor>();
    registerSequenceEditor<ControlSequenceEditor>();

    // Map Ordinal properties
    using Types = std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4,
                             dmat2, dmat3, dmat4, int, ivec2, ivec3, ivec4, unsigned int, uvec2,
                             uvec3, uvec4, size_t, size2_t, size3_t, size4_t>;
    util::for_each_type<Types>{}(Reghelper<OrdinalProperty>{}, *this);
    util::for_each_type<Types>{}(Reghelper<OrdinalRefProperty>{}, *this);

    util::for_each_type<std::tuple<float, double, int, unsigned int, size_t>>{}(
        Reghelper<MinMaxProperty>{}, *this);

    util::for_each_type<std::tuple<float, double, int, unsigned int, size_t, std::string>>{}(
        Reghelper<OptionProperty>{}, *this);

    util::for_each_type<std::tuple<BoolProperty, FileProperty, StringProperty>>{}(
        PropertyValueKeyframeReghelper{}, *this);

    registerPropertyTrackHelper<CameraProperty, CameraKeyframe>(*this);

    registerTrackToWidgetMap(ButtonTrack::classIdentifier(),
                             PropertyTrackWidgetQt::classIdentifier());

    registerTrackToWidgetMap(CallbackTrack::classIdentifier(),
                             PropertyTrackWidgetQt::classIdentifier());

    registerTrackToWidgetMap(ControlTrack::classIdentifier(),
                             ControlTrackWidgetQt::classIdentifier());

    registerTrackToSequenceEditorMap(ControlTrack::classIdentifier(),
                                     ControlSequenceEditor::classIdentifier());
}  // namespace inviwo

AnimationQtModule::~AnimationQtModule() {
    // Unregister everything from the factory since this module _owns_ the factory. This is
    // neccessary even though the base class destructor, i.e. animation::AnimationQtSupplier, takes
    // care of this. Otherwise the supplier will unregister the items _after_ the factory is
    // destroyed.
    //
    // Other modules do not have to do this!
    unRegisterAll();
}

animation::TrackWidgetQtFactory& AnimationQtModule::getTrackWidgetQtFactory() {
    return trackWidgetQtFactory_;
}
const animation::TrackWidgetQtFactory& AnimationQtModule::getTrackWidgetQtFactory() const {
    return trackWidgetQtFactory_;
}

animation::SequenceEditorFactory& AnimationQtModule::getSequenceEditorFactory() {
    return sequenceEditorFactory_;
}
const animation::SequenceEditorFactory& AnimationQtModule::getSequenceEditorFactory() const {
    return sequenceEditorFactory_;
}

}  // namespace inviwo
