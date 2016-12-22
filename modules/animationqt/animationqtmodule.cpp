/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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
#include <modules/animationqt/animationeditordockwidgetqt.h>

#include <modules/animation/animationmodule.h>
#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframe.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/properties/ordinalproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <warn/pop>

namespace inviwo {

// --- BOGUS DATA, USE ME FOR TESTING ---
using namespace animation;
using FloatKey = ValueKeyframe<float>;
using Vec3Key = ValueKeyframe<vec3>;

auto bogusFloatProp = std::make_unique<FloatProperty>("float_id", "Float property");
auto bogusVec3Prop = std::make_unique<FloatVec3Property>("vec3_id", "Vec3 property");
auto bogusAnim = std::make_unique<Animation>();
auto bogusController = std::make_unique<AnimationController>(bogusAnim.get());

void initBogus() {
    {
        KeyframeSequenceTyped<FloatKey> s1{{{Time(0), 1.f}, {Time(1), 2.f}, {Time(4), 5.f}},
                                           std::make_unique<LinearInterpolation<FloatKey>>()};
        KeyframeSequenceTyped<FloatKey> s2{{{Time(5), 2.f}, {Time(6), 1.f}},
                                           std::make_unique<LinearInterpolation<FloatKey>>()};

        auto track = std::make_unique<TrackProperty<FloatProperty, FloatKey>>(bogusFloatProp.get());
        track->add(s1);
        track->setName("Lame ass float track");
        bogusAnim->add(std::move(track));
    }

    {
        KeyframeSequenceTyped<Vec3Key> s1{{{Time(1), vec3(2)}, {Time(4), vec3(1)}},
                                          std::make_unique<LinearInterpolation<Vec3Key>>()};

        auto track =
            std::make_unique<TrackProperty<FloatVec3Property, Vec3Key>>(bogusVec3Prop.get());
        track->add(s1);
        track->setName("Lame ass vec3 track");
        bogusAnim->add(std::move(track));
    }
}
// --- END OF BOGUS ---

AnimationQtModule::AnimationQtModule(InviwoApplication* app) : InviwoModule(app, "AnimationQt") {

    if (auto win = utilqt::getApplicationMainWindow()) {
        initBogus();
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
            menu = win->menuBar()->addMenu(animationMenuName);
        }
        controller.setAnimation(bogusAnim.release());
        auto& controller =
            app->getModuleByType<AnimationModule>()->getAnimationManager().getAnimationController();
        auto editor =
            new animation::AnimationEditorDockWidgetQt(controller, "Animation Editor", win);
        menu->addAction(editor->toggleViewAction());
        editor->hide();
    }
}

AnimationQtModule::~AnimationQtModule() {

}

} // namespace
