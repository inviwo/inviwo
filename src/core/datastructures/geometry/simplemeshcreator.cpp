/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

namespace inviwo {

std::shared_ptr<SimpleMesh> SimpleMeshCreator::rectangularPrism(vec3 posLlf, vec3 posUrb,
                                                                vec3 texCoordLlf, vec3 texCoordUrb,
                                                                vec4 colorLlf, vec4 colorUrb) {
    auto recPrism = std::make_shared<SimpleMesh>();
    // Set identity matrix
    recPrism->setModelMatrix(mat4(1.f));
    // 8 corners
    recPrism->addVertex(posLlf, texCoordLlf, colorLlf);
    recPrism->addVertex(vec3(posLlf.x, posUrb.y, posLlf.z),
                        vec3(texCoordLlf.x, texCoordUrb.y, texCoordLlf.z),
                        vec4(colorLlf.x, colorUrb.y, colorLlf.z, colorLlf.w));
    recPrism->addVertex(vec3(posLlf.x, posUrb.y, posUrb.z),
                        vec3(texCoordLlf.x, texCoordUrb.y, texCoordUrb.z),
                        vec4(colorLlf.x, colorUrb.y, colorUrb.z, colorLlf.w));
    recPrism->addVertex(vec3(posLlf.x, posLlf.y, posUrb.z),
                        vec3(texCoordLlf.x, texCoordLlf.y, texCoordUrb.z),
                        vec4(colorLlf.x, colorLlf.y, colorUrb.z, colorLlf.w));
    recPrism->addVertex(vec3(posUrb.x, posLlf.y, posUrb.z),
                        vec3(texCoordUrb.x, texCoordLlf.y, texCoordUrb.z),
                        vec4(colorUrb.x, colorLlf.y, colorUrb.z, colorUrb.w));
    recPrism->addVertex(vec3(posUrb.x, posLlf.y, posLlf.z),
                        vec3(texCoordUrb.x, texCoordLlf.y, texCoordLlf.z),
                        vec4(colorUrb.x, colorLlf.y, colorLlf.z, colorUrb.w));
    recPrism->addVertex(vec3(posUrb.x, posUrb.y, posLlf.z),
                        vec3(texCoordUrb.x, texCoordUrb.y, texCoordLlf.z),
                        vec4(colorUrb.x, colorUrb.y, colorLlf.z, colorUrb.w));
    recPrism->addVertex(posUrb, texCoordUrb, colorUrb);
    // 14 indices (Triangle Strip)
    recPrism->setIndicesInfo(DrawType::Triangles, ConnectivityType::Strip);
    recPrism->addIndex(3);
    recPrism->addIndex(4);
    recPrism->addIndex(2);
    recPrism->addIndex(7);
    recPrism->addIndex(6);
    recPrism->addIndex(4);
    recPrism->addIndex(5);
    recPrism->addIndex(0);
    recPrism->addIndex(6);
    recPrism->addIndex(1);
    recPrism->addIndex(2);
    recPrism->addIndex(0);
    recPrism->addIndex(3);
    recPrism->addIndex(4);

    return recPrism;
}

std::shared_ptr<SimpleMesh> SimpleMeshCreator::parallelepiped(
    glm::vec3 pos, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 tex, glm::vec3 t1,
    glm::vec3 t2, glm::vec3 t3, glm::vec4 col, glm::vec4 c1, glm::vec4 c2, glm::vec4 c3) {
    auto ppd = std::make_shared<SimpleMesh>();
    // Set identity matrix
    ppd->setModelMatrix(mat4(1.f));

    //      2-----3
    //     /|    /|          y
    //    6-+---7 |          |
    //    | 0---+-1          o--x
    //    |/    |/          /
    //    4-----5          z
    //
    // See: http://www.cs.umd.edu/gvil/papers/av_ts.pdf
    // Triangle Strip: 0 1 4 5 7 1 3 0 2 4 6 7 2 3
    // 8 corners
    ppd->addVertex(pos, tex, col);                                               // (0,0,0)
    ppd->addVertex(pos + p1, tex + t1, col + c1);                                // (1,0,0)
    ppd->addVertex(pos + p2, tex + t2, col + c2);                                // (0,1,0)
    ppd->addVertex(pos + p1 + p2, tex + t1 + t2, col + c1 + c2);                 // (1,1,0)
    ppd->addVertex(pos + p3, tex + t3, col + c3);                                // (0,0,1)
    ppd->addVertex(pos + p1 + p3, tex + t1 + t3, col + c1 + c3);                 // (1,0,1)
    ppd->addVertex(pos + p2 + p3, tex + t2 + t3, col + c2 + c3);                 // (0,1,1)
    ppd->addVertex(pos + p1 + p2 + p3, tex + t1 + t2 + t3, col + c1 + c2 + c3);  // (1,1,1)
    // 14 indices (Triangle Strip)
    ppd->setIndicesInfo(DrawType::Triangles, ConnectivityType::Strip);
    ppd->addIndex(0);
    ppd->addIndex(1);
    ppd->addIndex(4);
    ppd->addIndex(5);
    ppd->addIndex(7);
    ppd->addIndex(1);
    ppd->addIndex(3);
    ppd->addIndex(0);
    ppd->addIndex(2);
    ppd->addIndex(4);
    ppd->addIndex(6);
    ppd->addIndex(7);
    ppd->addIndex(2);
    ppd->addIndex(3);
    return ppd;
}

std::shared_ptr<SimpleMesh> SimpleMeshCreator::rectangle(vec3 posLl, vec3 posUr) {
    auto rec = std::make_shared<SimpleMesh>();
    // Set identity matrix
    rec->setModelMatrix(mat4(1.f));
    vec3 texCoordLl(0, 0, 0);
    vec3 texCoordUr(1, 1, 0);
    vec4 colorLl(1, 1, 1, 1);
    vec4 colorUr(0, 1, 0, 1);
    // 4 corners
    rec->addVertex(posLl, texCoordLl, colorLl);
    rec->addVertex(vec3(posLl.x, posUr.y, posLl.z), vec3(texCoordLl.x, texCoordUr.y, texCoordLl.z),
                   vec4(colorLl.x, colorUr.y, colorLl.z, colorLl.w));
    rec->addVertex(vec3(posUr.x, posLl.y, posUr.z), vec3(texCoordLl.x, texCoordUr.y, texCoordUr.z),
                   vec4(colorLl.x, colorUr.y, colorUr.z, colorLl.w));
    rec->addVertex(posUr, texCoordUr, colorUr);
    // 4 indices (?)
    rec->setIndicesInfo(DrawType::Triangles, ConnectivityType::Strip);
    rec->addIndex(1);
    rec->addIndex(3);
    rec->addIndex(0);
    rec->addIndex(2);
    return rec;
}

std::shared_ptr<SimpleMesh> SimpleMeshCreator::sphere(float radius, unsigned int numLoops,
                                                      unsigned int segmentsPerLoop) {
    auto spheremesh = std::make_shared<SimpleMesh>();

    numLoops = std::max(4u, numLoops);
    segmentsPerLoop = std::max(8u, segmentsPerLoop);

    // Set identity matrix
    spheremesh->setModelMatrix(mat4(1.f));

    // Create Vertices
    auto normals = std::make_shared<Vec3BufferRAM>((numLoops + 1) * (segmentsPerLoop + 1));
    auto normalBuffer = std::make_shared<Buffer<vec3>>(normals);

    unsigned int pointsPerLine = segmentsPerLoop + 1;
    for (unsigned int i = 0; i <= numLoops; ++i) {
        for (unsigned int j = 0; j <= segmentsPerLoop; ++j) {
            float theta = (i * static_cast<float>(M_PI) / numLoops);

            if (i == numLoops) theta = static_cast<float>(M_PI);

            float phi = j * 2 * static_cast<float>(M_PI) / segmentsPerLoop;
            float sinTheta = std::sin(theta);
            float sinPhi = std::sin(phi);
            float cosTheta = std::cos(theta);
            float cosPhi = std::cos(phi);
            vec3 normal(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
            vec3 vert(normal * radius);
            vec3 texCoord(static_cast<float>(j) / segmentsPerLoop, static_cast<float>(i) / numLoops,
                          0.0f);
            spheremesh->addVertex(vert, texCoord, vec4(vert, 1.f));
            normals->set(i * pointsPerLine + j, normal);
        }
    }
    spheremesh->addBuffer(BufferType::NormalAttrib, normalBuffer);

    // Create Indices
    // compute indices
    spheremesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
    for (unsigned int y = 0; y < numLoops; ++y) {
        auto indices = std::make_shared<IndexBufferRAM>(pointsPerLine * 2);
        auto indexBuf = std::make_shared<IndexBuffer>(indices);

        unsigned int offset = y * pointsPerLine;
        std::size_t count = 0;
        for (unsigned int x = 0; x < pointsPerLine; ++x) {
            indices->set(count++, offset + x);
            indices->set(count++, offset + x + pointsPerLine);
        }

        spheremesh->addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip),
                                indexBuf);
    }

    return spheremesh;
}

std::shared_ptr<SimpleMesh> SimpleMeshCreator::sphere(float radius, unsigned int numLoops,
                                                      unsigned int segmentsPerLoop, vec4 color) {
    auto spheremesh = std::make_shared<SimpleMesh>();

    numLoops = std::max(4u, numLoops);
    segmentsPerLoop = std::max(8u, segmentsPerLoop);

    // Set identity matrix
    spheremesh->setModelMatrix(mat4(1.f));

    // Create Vertices
    auto normals = std::make_shared<Vec3BufferRAM>((numLoops + 1) * (segmentsPerLoop + 1));
    auto normalBuffer = std::make_shared<Buffer<vec3>>(normals);

    unsigned int pointsPerLine = segmentsPerLoop + 1;
    for (unsigned int i = 0; i <= numLoops; ++i) {
        for (unsigned int j = 0; j <= segmentsPerLoop; ++j) {
            float theta =
                (i * static_cast<float>(M_PI) /
                 numLoops);  // + ((static_cast<float>(M_PI) * j) / (segmentsPerLoop * numLoops));

            if (i == numLoops) theta = static_cast<float>(M_PI);

            float phi = j * 2 * static_cast<float>(M_PI) / segmentsPerLoop;
            float sinTheta = std::sin(theta);
            float sinPhi = std::sin(phi);
            float cosTheta = std::cos(theta);
            float cosPhi = std::cos(phi);
            vec3 normal(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
            vec3 vert(normal * radius);
            vec3 texCoord(static_cast<float>(j) / segmentsPerLoop, static_cast<float>(i) / numLoops,
                          0.0f);
            spheremesh->addVertex(vert, texCoord, color);
            normals->set(i * pointsPerLine + j, normal);
        }
    }
    spheremesh->addBuffer(BufferType::NormalAttrib, normalBuffer);

    // Create Indices
    // compute indices
    spheremesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
    for (unsigned int y = 0; y < numLoops; ++y) {
        auto indices = std::make_shared<IndexBufferRAM>(pointsPerLine * 2);
        auto indexBuf = std::make_shared<IndexBuffer>(indices);

        unsigned int offset = y * pointsPerLine;
        std::size_t count = 0;
        for (unsigned int x = 0; x < pointsPerLine; ++x) {
            indices->set(count++, offset + x);
            indices->set(count++, offset + x + pointsPerLine);
        }

        spheremesh->addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip),
                                indexBuf);
    }

    return spheremesh;
}

std::shared_ptr<SimpleMesh> SimpleMeshCreator::plane(glm::vec3 pos, glm::vec2 extent,
                                                     unsigned int meshResX, unsigned int meshResY) {
    auto plane = std::make_shared<SimpleMesh>();
    // Set identity matrix
    plane->setModelMatrix(mat4(1.f));

    meshResX = std::max(1u, meshResX);
    meshResY = std::max(1u, meshResY);

    glm::vec3 p0(pos - glm::vec3(extent, 0.0f) * 0.5f);

    glm::vec3 texCoordDelta(1.0f / glm::vec2(meshResX, meshResY), 0.0f);
    glm::vec3 stepDelta(extent.x * texCoordDelta.x, extent.y * texCoordDelta.y, 0.0f);

    unsigned int pointsPerLine = meshResX + 1;

    const glm::vec4 color(0.6f, 0.6f, 0.6f, 1.0f);

    auto normals = std::make_shared<Vec3BufferRAM>((meshResX + 1) * (meshResY + 1));
    auto normalBuffer = std::make_shared<Buffer<vec3>>(normals);

    for (unsigned int y = 0; y <= meshResY; ++y) {
        for (unsigned int x = 0; x <= meshResX; ++x) {
            glm::vec3 tCoord(texCoordDelta * glm::vec3(x, y, 0.0f));
            plane->addVertex(p0 + stepDelta * glm::vec3(x, y, 0.0f), tCoord, color);
            normals->set(y * pointsPerLine + x, vec3(0.0f, 0.0f, 1.0f));
        }
    }
    plane->addBuffer(BufferType::NormalAttrib, normalBuffer);

    // compute indices
    plane->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
    for (unsigned int y = 0; y < meshResY; ++y) {
        auto indices = std::make_shared<IndexBufferRAM>(pointsPerLine * 2);
        auto indexBuf = std::make_shared<IndexBuffer>(indices);

        unsigned int offset = y * pointsPerLine;
        std::size_t count = 0;
        for (unsigned int x = 0; x < pointsPerLine; ++x) {
            indices->set(count++, offset + x);
            indices->set(count++, offset + x + pointsPerLine);
        }

        plane->addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indexBuf);
    }

    return plane;
}

}  // namespace
