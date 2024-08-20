/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <modules/opactopt/rendering/approximation.h>

namespace inviwo {
namespace Approximations {

double choose(double n, double k) {
    if (k == 0.0) return 1.0;
    return (n * choose(n - 1, k - 1)) / k;
}

std::vector<float> generateLegendreCoefficients() {
    double maxDegree = Approximations::approximations.at("legendre").maxCoefficients - 1;
    std::vector<float> coeffs;
    coeffs.reserve((maxDegree + 1) * (maxDegree + 2) / 2);

    for (double n = 0; n <= maxDegree; n += 1.0f) {
        for (double k = 0; k <= n; k += 1.0f) {
            double res = choose(n, k) * choose(n + k, k);
            res *= (int)(n + k) % 2 == 0 ? 1.0f : -1.0f;
            coeffs.push_back((double)res);
        }
    }

    return coeffs;
}

DebugBuffer::DebugBuffer()
    : debugApproximationCoeffsBuffer_{Approximations::approximations.at("fourier").maxCoefficients *
                                          sizeof(float) * 2,
                                      GLFormats::getGLFormat(GL_FLOAT, 1), GL_STATIC_DRAW,
                                      GL_SHADER_STORAGE_BUFFER}
    , debugFragmentsBuffer_{sizeof(int) + 16384 * 2 * sizeof(float),
                            GLFormats::getGLFormat(GL_FLOAT, 1), GL_STATIC_DRAW,
                            GL_SHADER_STORAGE_BUFFER} {}

void DebugBuffer::initialiseDebugBuffer() {
    ready = false;

    debugApproximationCoeffsBuffer_ = BufferObject(
        Approximations::approximations.at("fourier").maxCoefficients * sizeof(float) * 2,
        GLFormats::getGLFormat(GL_FLOAT, 1), GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER);
    debugFragmentsBuffer_ =
        BufferObject(sizeof(int) + 16384 * 2 * sizeof(float), GLFormats::getGLFormat(GL_FLOAT, 1),
                     GL_STATIC_DRAW, GL_SHADER_STORAGE_BUFFER);

    float val = 0.0f;
    debugFragmentsBuffer_.bind();
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &val);

    debugApproximationCoeffsBuffer_.bindBase(10);
    debugFragmentsBuffer_.bindBase(11);
}

void DebugBuffer::retrieveDebugInfo(int nIsc, int nOdc) {
    ready = true;

    // Fetch coefficient data
    debugApproximationCoeffsBuffer_.bind();
    debugCoeffs_.importanceSumCoeffs.resize(nIsc);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, nIsc * sizeof(float),
                       &debugCoeffs_.importanceSumCoeffs[0]);
    debugCoeffs_.opticalDepthCoeffs.resize(nOdc);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, nIsc * sizeof(float), nOdc * sizeof(float),
                       &debugCoeffs_.opticalDepthCoeffs[0]);

    int nFragments;
    debugFragmentsBuffer_.bind();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &nFragments);
    nFragments = glm::min(nFragments, 16384);
    debugFrags.resize(nFragments);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 2 * nFragments * sizeof(float),
                       &debugFrags[0]);
}

void DebugBuffer::exportDebugInfo(std::filesystem::path path, const ApproximationProperties ap,
                                  int q, int r, int lambda) {
    std::ofstream file(path);

    file << debugCoeffs_.importanceSumCoeffs.size() << std::endl;
    file << debugCoeffs_.opticalDepthCoeffs.size() << std::endl;
    file << debugFrags.size() << std::endl;
    file << q << std::endl;
    file << r << std::endl;
    file << lambda << std::endl;
    file << ap.name << std::endl;
    for (float isc : debugCoeffs_.importanceSumCoeffs) file << isc << std::endl;
    for (float odc : debugCoeffs_.opticalDepthCoeffs) file << odc << std::endl;
    for (auto frag : debugFrags) {
        file << frag.depth << " " << frag.importance << std::endl;
    }

    LogInfo("Debug info exported to " << path);
}

}  // namespace Approximations
}  // namespace inviwo
