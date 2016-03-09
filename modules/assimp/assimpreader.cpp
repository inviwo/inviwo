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

#include <inviwo/core/util/exception.h>
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
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>

#include <warn/pop>

#include <array>
#include <ctime>

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

    // manually add .iv as known file extension (assimp is using signatures not (only) file
    // extensions)
    addExtension(FileExtension("iv", "VRML1 Importer .iv ('dirty' hack)."));
}

AssimpReader* AssimpReader::clone() const { return new AssimpReader(*this); }


std::shared_ptr<Mesh> AssimpReader::readData(const std::string filePath) {
    Assimp::Importer importer;

    std::clock_t start_readmetadata = std::clock();

    // logging
    Assimp::DefaultLogger::create("AssimpImportLog.txt", Assimp::Logger::LogSeverity::NORMAL, 0);
    Assimp::DefaultLogger::get()->attachStream(new InviwoAssimpLogStream(LogLevel::Warn),
                                               Assimp::Logger::ErrorSeverity::Warn);
    Assimp::DefaultLogger::get()->attachStream(new InviwoAssimpLogStream(LogLevel::Error),
                                               Assimp::Logger::ErrorSeverity::Err);


    //#define AI_CONFIG_PP_SBP_REMOVE "aiPrimitiveType_POINTS | aiPrimitiveType_LINES"
    //#define AI_CONFIG_PP_FD_REMOVE 1

    const aiScene* scene = importer.ReadFile(
        filePath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
                      aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices |
                      aiProcess_ValidateDataStructure | aiProcess_ImproveCacheLocality |
                      aiProcess_RemoveRedundantMaterials | aiProcess_FindInvalidData |
                      aiProcess_OptimizeMeshes);
    // aiProcess_OptimizeGraph is incompatible to aiProcess_PreTransformVertices
    // aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_TransformUVCoords |
    // aiProcess_FindInstances
    // aiProcess_OptimizeGraph | aiProcess_SortByPType | aiProcess_FindDegenerates |

    std::clock_t start_convert = std::clock();
    Assimp::DefaultLogger::get()->debug(
        "time to load: " +
        std::to_string(double((std::clock() - start_readmetadata) / CLOCKS_PER_SEC)));

    if (!scene) {
        throw DataReaderException(importer.GetErrorString(), IvwContext);
    }

    // at least one mesh
    if (0 == scene->mNumMeshes) {
        throw DataReaderException("there are no meshes!", IvwContext);
    }

    // because we use aiProcess_PreTransformVertices we can safely ignore the scenegraph,
    // nevertheless we must not ignore all the other differences:
    // to load multiple assimp meshes (from one model) in one inviwo mesh we add a vertex counter to
    // correctly calculate the indices, assume all meshes have normal data or not (both options can
    // be done with preprocessing);
    // fill texture and color channels with garbage/padding data if the channel count differs
    // between meshes

    uint32_t vertex_offset = 0;
    size_t color_channels = 0;
    size_t texture_channels = 0;
    bool use_normals = true;
    bool use_materials = scene->HasMaterials();

    // we have at least one mesh, so get its geometry type
    DrawType dt = DrawType::NotSpecified;

    size_t fst_primitive_type = size_t{scene->mMeshes[0]->mPrimitiveTypes};
    if (fst_primitive_type == aiPrimitiveType_POINT) {
        dt = DrawType::Points;
    } else if (fst_primitive_type == aiPrimitiveType_LINE) {
        dt = DrawType::Lines;
    } else if (fst_primitive_type == aiPrimitiveType_TRIANGLE) {
        dt = DrawType::Triangles;
    }

    // get the configuration
    for (size_t i = 0; i < size_t{scene->mNumMeshes}; ++i) {
        aiMesh* m = scene->mMeshes[i];

        color_channels = std::max(size_t{m->GetNumColorChannels()}, color_channels);
        texture_channels = std::max(size_t{m->GetNumUVChannels()}, texture_channels);

        if (false == m->HasNormals()) {
            use_normals = false;
        }

        // check if all meshes have the same geometry type
        if (m->mPrimitiveTypes != fst_primitive_type) {
            dt = DrawType::NotSpecified;
        }
    }

    // if we have a material available, ensure that it's used
    if (use_materials) {
        color_channels = std::max(color_channels, size_t{1});
        Assimp::DefaultLogger::get()->debug("model has materials.");
    }

    // create Inviwos data structures for the model
    auto mesh = std::make_shared<Mesh>();

    auto prep = std::make_shared<Vec3BufferRAM>();
    auto pbuff = std::make_shared<Buffer<vec3>>(prep);

    std::shared_ptr<Vec3BufferRAM> nrep;
    std::shared_ptr<Buffer<vec3>> nbuff;

    if (use_normals) {
        nrep = std::make_shared<Vec3BufferRAM>();
        nbuff = std::make_shared<Buffer<vec3>>(nrep);
    }

    std::array<std::shared_ptr<Buffer<vec4>>, AI_MAX_NUMBER_OF_COLOR_SETS> cbuff;
    std::array<std::shared_ptr<Vec4BufferRAM>, AI_MAX_NUMBER_OF_COLOR_SETS> crep;

    for (size_t i = 0; i < color_channels; ++i) {
        crep[i] = std::make_shared<Vec4BufferRAM>();
        cbuff[i] = std::make_shared<Buffer<vec4>>(crep[i]);
    }

    std::array<std::shared_ptr<Buffer<vec3>>, AI_MAX_NUMBER_OF_TEXTURECOORDS> tbuff;
    std::array<std::shared_ptr<Vec3BufferRAM>, AI_MAX_NUMBER_OF_TEXTURECOORDS> trep;

    for (size_t i = 0; i < texture_channels; ++i) {
        trep[i] = std::make_shared<Vec3BufferRAM>();
        tbuff[i] = std::make_shared<Buffer<vec3>>(trep[i]);
    }

    auto ibuff = std::make_shared<IndexBufferRAM>();
    auto inds = std::make_shared<IndexBuffer>(ibuff);

    // iterate over the meshes and fill the data structures
    for (size_t i = 0; i < size_t{scene->mNumMeshes}; ++i) {
        aiMesh* m = scene->mMeshes[i];

        // positions
        for (size_t j = 0; j < m->mNumVertices; ++j) {
            aiVector3D v = m->mVertices[j];
            prep->add(vec3(v.x, v.y, v.z));
        }

        // normals
        if (use_normals) {
            for (size_t j = 0; j < m->mNumVertices; ++j) {
                aiVector3D n = m->mNormals[j];
                nrep->add(vec3(n.x, n.y, n.z));
            }
        }

        // colors
        vec4 padding_color(0.6, 0.6, 0.6, 1.0);
        const float material_influence = 0.5f;

        if (use_materials) {
            aiMaterial* mat = scene->mMaterials[m->mMaterialIndex];

            aiColor4D ambient;
            aiColor4D diffuse;
            bool use_ambient = false;

            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &ambient)) {
                padding_color = vec4(ambient.r, ambient.g, ambient.b, ambient.a);
                use_ambient = true;
            }

            if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
                vec4 dcol = vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
                if (use_ambient) {
                    padding_color += dcol;
                    padding_color *= 0.5f;
                } else {
                    padding_color = dcol;
                }
            }

            vec4 material_influence_color = padding_color * material_influence;

            for (size_t l = 0; l < size_t{m->GetNumColorChannels()}; ++l) {
                for (size_t j = 0; j < size_t{m->mNumVertices}; ++j) {
                    aiColor4D c = m->mColors[l][j];
                    crep[l]->add(vec4(c.r, c.g, c.b, c.a) * (1.0f - material_influence) +
                                 material_influence_color);
                }
            }
        } else {
            for (size_t l = 0; l < size_t{m->GetNumColorChannels()}; ++l) {
                for (size_t j = 0; j < size_t{m->mNumVertices}; ++j) {
                    aiColor4D c = m->mColors[l][j];
                    crep[l]->add(vec4(c.r, c.g, c.b, c.a));
                }
            }
        }

        // fill not existing color channels with padding data
        for (size_t l = size_t{m->GetNumColorChannels()}; l < color_channels; ++l) {
            for (size_t j = 0; j < size_t{m->mNumVertices}; ++j) {
                crep[l]->add(padding_color);
            }
        }

        // texture coordinates
        for (size_t l = 0; l < m->GetNumUVChannels(); ++l) {
            for (size_t j = 0; j < m->mNumVertices; ++j) {
                aiVector3D t = m->mTextureCoords[l][j];
                trep[l]->add(vec3(t.x, t.y, t.z));
            }
        }

        for (size_t l = size_t{m->GetNumUVChannels()}; l < texture_channels; ++l) {
            for (size_t j = 0; j < size_t{m->mNumVertices}; ++j) {
                trep[l]->add(vec3(0.0, 0.0, 0.0));
            }
        }

        // indices
        for (size_t j = 0; j < m->mNumFaces; ++j) {
            aiFace face = m->mFaces[j];
            for (size_t k = 0; k < face.mNumIndices; ++k) {
                ibuff->add(vertex_offset + face.mIndices[k]);
            }
        }

        vertex_offset += m->mNumVertices;
    }

    // add the data to the mesh
    mesh->addBuffer(BufferType::PositionAttrib, pbuff);

    if (use_normals) {
        mesh->addBuffer(BufferType::NormalAttrib, nbuff);
    }

    for (size_t i = 0; i < color_channels; ++i) {
        mesh->addBuffer(BufferType::ColorAttrib, cbuff[i]);
    }

    for (size_t i = 0; i < texture_channels; ++i) {
        mesh->addBuffer(BufferType::TexcoordAttrib, tbuff[i]);
    }

    mesh->addIndicies(Mesh::MeshInfo(dt, ConnectivityType::None), inds);

    std::clock_t now = std::clock();
    Assimp::DefaultLogger::get()->debug(
        "time to convert: " + std::to_string(double((now - start_convert) / CLOCKS_PER_SEC)));
    Assimp::DefaultLogger::get()->debug(
        "overall time to import: " +
        std::to_string(double((now - start_readmetadata) / CLOCKS_PER_SEC)));

    Assimp::DefaultLogger::kill();

    return mesh;
}

}  // namespace
