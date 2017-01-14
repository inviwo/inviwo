/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>


#include <modules/animation/datastructures/easing.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>

namespace inviwo {
namespace animation {

TEST(AnimationTests, Easing) {
    /*
    // Get array of easings
    std::vector<std::pair<Easing::EEasingType, std::string> > AllEasings = {
        {Easing::EEasingType::None, "None"},
        {Easing::EEasingType::Linear, "Linear"},
        {Easing::EEasingType::InQuadratic, "InQuadratic"},
        {Easing::EEasingType::InCubic, "InCubic"},
        {Easing::EEasingType::InQuartic, "InQuartic"},
        {Easing::EEasingType::InQuintic, "InQuintic"},
        {Easing::EEasingType::OutQuadratic, "OutQuadratic"},
        {Easing::EEasingType::OutCubic, "OutCubic"},
        {Easing::EEasingType::OutQuartic, "OutQuartic"},
        {Easing::EEasingType::OutQuintic, "OutQuintic"},
        {Easing::EEasingType::InOutQuadratic, "InOutQuadratic"},
        {Easing::EEasingType::InOutCubic, "InOutCubic"},
        {Easing::EEasingType::InOutQuartic, "InOutQuartic"},
        {Easing::EEasingType::InOutQuintic, "InOutQuintic"},
        {Easing::EEasingType::InSine, "InSine"},
        {Easing::EEasingType::OutSine, "OutSine"},
        {Easing::EEasingType::InOutSine, "InOutSine"},
        {Easing::EEasingType::InExp, "InExp"},
        {Easing::EEasingType::OutExp, "OutExp"},
        {Easing::EEasingType::InOutExp, "InOutExp"},
        {Easing::EEasingType::InCircular, "InCircular"},
        {Easing::EEasingType::OutCircular, "OutCircular"},
        {Easing::EEasingType::InOutCircular, "InOutCircular"},
        {Easing::EEasingType::InBack, "InBack"},
        {Easing::EEasingType::OutBack, "OutBack"},
        {Easing::EEasingType::InOutBack, "InOutBack"},
        {Easing::EEasingType::InElastic, "InElastic"},
        {Easing::EEasingType::OutElastic, "OutElastic"},
        {Easing::EEasingType::InOutElastic, "InOutElastic"},
        {Easing::EEasingType::InBounce, "InBounce"},
        {Easing::EEasingType::OutBounce, "OutBounce"},
        {Easing::EEasingType::InOutBounce, "InOutBounce"}};

    // Create some sample points
    std::vector<double> X(1001);
    for (size_t i = 0; i < X.size(); i++) {
        X[i] = double(i) / (X.size() - 1);
    }

    // Function Values
    std::vector<double> Y(X);

    
    // Run over all easings
    for (const auto& ThisEasing : AllEasings) {
        // Compute
        for (size_t i = 0; i < X.size(); i++) {
            Y[i] = Easing::Ease(X[i], ThisEasing.first);
        }

        // Open file and save
        std::ofstream outfile(ThisEasing.second + ".py");
        if (outfile.is_open()) {
            outfile << "import numpy as np\nimport matplotlib.pyplot as plt\n\n";

            outfile << "x = np.array([";
            for (size_t i = 0; i < X.size(); i++) {
                outfile << X[i] << ", ";
            }
            outfile << "])\n";

            outfile << "y = np.array([";
            for (size_t i = 0; i < Y.size(); i++) {
                outfile << Y[i] << ", ";
            }
            outfile << "])\n\n";

            outfile << "fig, ax = plt.subplots()\nplt.plot([0,1], [1,1], '0.75', "
                       "linewidth=2)\nplt.plot([0,1], [0,0], '0.75', linewidth=2)\nplt.plot(x, y, "
                       "'r', linewidth=2)\n\nplt.xlim([0, 1.1])\nplt.ylim([-0.15, "
                       "1.15])\n\nplt.figtext(0.9, 0.07, '$t$')\nplt.figtext(0.1, 0.9, "
                       "'$t\\'$')\n\nax.spines['right'].set_visible(False)\nax.spines['top'].set_"
                       "visible(False)\nax.xaxis.set_ticks_position('bottom')\nax.yaxis.set_ticks_"
                       "position('left')\n\nax.set_xticks((0, 0.5, 1))\nax.set_xticklabels((0, "
                       "0.5, 1))\nax.set_yticks((0, 0.5, 1))\nax.set_yticklabels((0, 0.5, "
                       "1))\n\nax.set_aspect('equal', 'datalim')\n\n";

            outfile << "plt.figtext(0.5, 0.825, '" << ThisEasing.second
                    << "', horizontalalignment='center', size='large')\n\n";

            outfile << "plt.savefig('" << ThisEasing.second
                    << ".png', bbox_inches='tight', dpi=300)\n";
            outfile.close();
        }
    }
    */

    EXPECT_EQ(1, 1);
}

}  // namespace

}  // namespace
