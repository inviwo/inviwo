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

#include "assimpreader.h"

#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/io/datareaderexception.h>

#include <warn/push>
#include <warn/ignore/all>
#include <assimp/Importer.hpp>   // C++ importer interface
#include <assimp/scene.h>        // Output data structure
#include <assimp/postprocess.h>  // Post processing flags
#include <assimp/types.h>
#include <assimp/importerdesc.h>
#include <warn/pop>

namespace inviwo {

AssimpReader::AssimpReader() : DataReaderType<Mesh>() {
    aiString str{};
    Assimp::Importer importer{};

    size_t readers = importer.GetImporterCount();
    for (size_t i = 0; i < readers; ++i) {
        const aiImporterDesc* desc = importer.GetImporterInfo(i);
        for (std::string e : splitString(std::string(desc->mFileExtensions), ' ')) {
            addExtension(FileExtension(e, desc->mName));
        }
    }
}

AssimpReader* AssimpReader::clone() const { return new AssimpReader(*this); }

std::shared_ptr<Mesh> AssimpReader::readData(const std::string filePath) {
    Assimp::Importer importer;

    // And have it read the given file with some example post processing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more post processing than we do in this example.
    const aiScene* scene = importer.ReadFile(
        filePath, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                      aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene) throw DataReaderException(importer.GetErrorString(), IvwContext);

    auto mesh = std::make_shared<Mesh>();

    // Only support reading one mesh from the scene. 
    for (size_t i = 0; i < std::min(size_t{1}, size_t{scene->mNumMeshes}); ++i) {
        aiMesh* m = scene->mMeshes[i];

        auto prep = std::make_shared<Vec3BufferRAM>(m->mNumVertices);
        auto pbuff = std::make_shared<Buffer<vec3>>(prep);

        for (size_t j = 0; j < m->mNumVertices; ++j) {
            aiVector3D v = m->mVertices[j];
            (*prep)[j] = vec3(v.x, v.y, v.z);
        }
        mesh->addBuffer(BufferType::POSITION_ATTRIB, pbuff);
   
        for (size_t l = 0; l < m->GetNumUVChannels(); ++l) {
            auto trep = std::make_shared<Vec3BufferRAM>(m->mNumVertices);
            auto tbuff = std::make_shared<Buffer<vec3>>(trep);
            for (size_t j = 0; j < m->mNumVertices; ++j) {
                aiVector3D t = m->mTextureCoords[l][j];
                (*trep)[j] = vec3(t.x, t.y, t.z);
            }
            mesh->addBuffer(BufferType::TEXCOORD_ATTRIB, tbuff);
        }

        for (size_t l = 0; l < m->GetNumColorChannels(); ++l) {
            auto crep = std::make_shared<Vec4BufferRAM>(m->mNumVertices);
            auto cbuff = std::make_shared<Buffer<vec4>>(crep);
            
            for (size_t j = 0; j < m->mNumVertices; ++j) {
                aiColor4D c = m->mColors[l][j];
                (*crep)[j] = vec4(c.r, c.g, c.b, c.a);
            }
            mesh->addBuffer(BufferType::COLOR_ATTRIB, cbuff);
        }

        if (m->HasNormals()) {
            auto nrep = std::make_shared<Vec3BufferRAM>(m->mNumVertices);
            auto nbuff = std::make_shared<Buffer<vec3>>(nrep);
            
            for (size_t j = 0; j < m->mNumVertices; ++j) {
                aiVector3D n = m->mNormals[j];
                (*nrep)[j] = vec3(n.x, n.y, n.z);
            }
            mesh->addBuffer(BufferType::NORMAL_ATTRIB, nbuff);
        }

        auto ibuff = std::make_shared<IndexBufferRAM>();
        auto inds = std::make_shared<IndexBuffer>(ibuff);
        
        for (size_t j = 0; j < m->mNumFaces; ++j) {
            aiFace face = m->mFaces[j];
            for (size_t k = 0; k < face.mNumIndices; ++k) {
                ibuff->add(face.mIndices[k]);
            }
        }

        DrawType dt = DrawType::NOT_SPECIFIED;
        if (m->mPrimitiveTypes == aiPrimitiveType_POINT) {
            dt = DrawType::POINTS;
        } else if (m->mPrimitiveTypes == aiPrimitiveType_LINE) {
            dt = DrawType::LINES;
        } else if (m->mPrimitiveTypes == aiPrimitiveType_TRIANGLE) {
            dt = DrawType::TRIANGLES;
        }
        mesh->addIndicies(Mesh::MeshInfo(dt, ConnectivityType::NONE), inds);
    }

    return mesh;
}

}  // namespace
