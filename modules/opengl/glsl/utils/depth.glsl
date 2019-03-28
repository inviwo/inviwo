/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifndef IVW_DEPTH_GLSL
#define IVW_DEPTH_GLSL

float convertScreenToEye(CameraParameters camera, float depthScreen) {
    float depthNDC = 2.0 * depthScreen - 1.0;
    float depthEye = 2.0 * camera.nearPlane * camera.farPlane /
                     (camera.farPlane + camera.nearPlane - depthNDC * (camera.farPlane - camera.nearPlane));
    return depthEye;
}

float convertEyeToScreen(CameraParameters camera, float depthEye) {
    float A = -(camera.farPlane+camera.nearPlane)/(camera.farPlane-camera.nearPlane);
    float B = (-2.0*camera.farPlane*camera.nearPlane) / (camera.farPlane-camera.nearPlane);
    float depthScreen = 0.5*(-A*depthEye + B) / depthEye + 0.5;
    return depthScreen;
}

float calculateDepthValue(CameraParameters camera, float t, float entryDepthScreen, float exitDepthScreen) {
    // to calculate the correct depth values, which are not linear in the deph buffer,
    // we must first convert our screen space coordinates into eye coordinates and interpolate there.
    // transform into eye space
    float entryDepthEye = convertScreenToEye(camera, entryDepthScreen);
    float exitDepthEye  = convertScreenToEye(camera, exitDepthScreen);
    // compute the depth value in clip space
    float resultEye = entryDepthEye + t * (exitDepthEye - entryDepthEye);
    // transform back to screen space
    float resultScreen = convertEyeToScreen(camera, resultEye);
    return resultScreen;
}



float calculateTValueFromDepthValue(CameraParameters camera, float depth, float entryDepthScreen, float exitDepthScreen) {
    // to calculate the correct depth values, which are not linear in the deph buffer,
    // we must first convert our screen space coordinates into eye coordinates and interpolate there.
    // transform into eye space
    float entryDepthEye = convertScreenToEye(camera, entryDepthScreen);
    float exitDepthEye  = convertScreenToEye(camera, exitDepthScreen);
    float depthEye  = convertScreenToEye(camera, depth);
    // compute the depth value in clip space
    return (depthEye - entryDepthEye) / (exitDepthEye - entryDepthEye);
}


float convertDepthViewToClip(CameraParameters camera, float z) {
    // convert linear depth from view coordinates to non-linear clip coords [-1,1]
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return (Zf + Zn) / (Zf - Zn) + (-2.0 * Zf * Zn) / (z * (Zf - Zn));
}

float convertDepthClipToView(CameraParameters camera, float z) {
    // convert non-linear depth from clip coords [-1,1] back to linear view coords (-inf,inf)
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return 2.0 * Zn * Zf / (Zf + Zn - z * (Zf - Zn));
}

float convertDepthScreenToView(CameraParameters camera, float z) {
    // convert non-linear depth from screen coords [0,1] back to linear view coords (-inf,inf)
    float Zn = camera.nearPlane;
    float Zf = camera.farPlane;

    return Zn*Zf / (Zf - z*(Zf - Zn));
}

float convertDepthViewToScreen(CameraParameters camera, float z) {
    // convert linear depth from view coordinates to non-linear screen coords [0,1]
    return (convertDepthViewToClip(camera, z) + 1.0) * 0.5;
}

#endif // IVW_DEPTH_GLSL