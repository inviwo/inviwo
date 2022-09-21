/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/assimp/assimpreader.h>

#include <inviwo/core/datastructures/buffer/buffer.h>                   // for Buffer, IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for Vec3BufferRAM
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for BufferType, DrawType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh, Mesh::Buffe...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/io/datareader.h>                                  // for DataReaderType
#include <inviwo/core/io/datareaderexception.h>                         // for DataReaderException
#include <inviwo/core/util/fileextension.h>                             // for FileExtension
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4
#include <inviwo/core/util/logcentral.h>                                // for LogVerbosity, Log...
#include <inviwo/core/util/sourcecontext.h>                             // for IVW_CONTEXT
#include <inviwo/core/util/stringconversion.h>                          // for splitStringView

#include <warn/push>
#include <warn/ignore/all>

#include <assimp/DefaultLogger.hpp>                                     // for DefaultLogger
#include <assimp/Importer.hpp>                                          // for Importer
#include <assimp/LogStream.hpp>                                         // for LogStream
#include <assimp/Logger.hpp>                                            // for Logger, Logger::E...
#include <assimp/color4.h>                                              // for aiColor4D
#include <assimp/importerdesc.h>                                        // for aiImporterDesc
#include <assimp/material.h>                                            // for aiGetMaterialColor
#include <assimp/mesh.h>                                                // for aiMesh, aiFace
#include <assimp/postprocess.h>                                         // for aiProcess_FindInv...
#include <assimp/scene.h>                                               // for aiScene
#include <assimp/types.h>                                               // for AI_SUCCESS, aiString
#include <assimp/vector3.h>                                             // for aiVector3D
#include <glm/vec4.hpp>                                                 // for operator*, operator+

#include <warn/pop>

#include <algorithm>                                                    // for max
#include <array>                                                        // for array, array<>::v...
#include <cstdint>                                                      // for uint32_t
#include <cstring>                                                      // for strlen
#include <ctime>                                                        // for size_t, clock
#include <string>                                                       // for basic_string<>::v...
#include <type_traits>                                                  // for remove_extent_t
#include <unordered_map>                                                // for unordered_map
#include <unordered_set>                                                // for unordered_set
#include <vector>                                                       // for vector

namespace inviwo {

/**
 * \brief Assimp LogStream => Inviwo LogCentral
 *
 *  Derive Assimp::LogStream to forward logged messages from the library to Inviwos LogCentral.
 */
class InviwoAssimpLogStream : public Assimp::LogStream {
private:
    LogLevel loglevel_;
    const std::string fileName_;

public:
    InviwoAssimpLogStream(LogLevel ploglevel, std::string_view filename = "")
        : loglevel_{ploglevel}, fileName_{filename} {}
    virtual ~InviwoAssimpLogStream() = default;

    void write(const char* message) {
        if (strlen(message) == 0) return;
        std::string tmp(message);
        while ('\n' == tmp.back()) tmp.pop_back();
        if (fileName_.size() > 0) {
            tmp += " (" + fileName_ + ")";
        }

        LogCentral::getPtr()->log("AssimpReader", loglevel_, LogAudience::User, __FILE__,
                                  "inviwo::AssimpReader::readData", 0, tmp);
    }
};

AssimpReader::AssimpReader()
    : DataReaderType<Mesh>()
    , logLevel_(AssimpLogLevel::Warn)
    , verboseLog_(false)
    , fixInvalidData_(false) {
    aiString str{};
    Assimp::Importer importer{};

    size_t readers = importer.GetImporterCount();
    for (size_t i = 0; i < readers; ++i) {
        const aiImporterDesc* desc = importer.GetImporterInfo(i);
        for (std::string_view e :
             util::splitStringView(std::string_view(desc->mFileExtensions), ' ')) {
            addExtension(FileExtension(e, desc->mName));
        }
    }

    // manually add .iv as known file extension (assimp is using signatures not (only) file
    // extensions)
    addExtension(FileExtension("iv", "VRML1 Importer .iv ('dirty' hack)."));
}

AssimpReader* AssimpReader::clone() const { return new AssimpReader(*this); }

void AssimpReader::setLogLevel(AssimpLogLevel level, bool verbose) {
    logLevel_ = level;
    verboseLog_ = verbose;
}

AssimpLogLevel AssimpReader::getLogLevel() const { return logLevel_; }

void AssimpReader::setFixInvalidDataFlag(bool enable) { fixInvalidData_ = enable; }

bool AssimpReader::getFixInvalidDataFlag() const { return fixInvalidData_; }

std::shared_ptr<Mesh> AssimpReader::readData(std::string_view filePath) {
    Assimp::Importer importer;

    std::clock_t start_readmetadata = std::clock();

    // logging
    bool logging = (logLevel_ != AssimpLogLevel::None);
    if (logging) {
        Assimp::Logger::LogSeverity logSeverity = verboseLog_ ? Assimp::Logger::LogSeverity::VERBOSE
                                                              : Assimp::Logger::LogSeverity::NORMAL;
        Assimp::DefaultLogger::create("AssimpImportLog.txt", logSeverity, 0);
        // if logging is enabled, errors will always be logged
        Assimp::DefaultLogger::get()->attachStream(
            new InviwoAssimpLogStream(LogLevel::Error, filePath),
            Assimp::Logger::ErrorSeverity::Err);
        if (logLevel_ >= AssimpLogLevel::Warn) {
            Assimp::DefaultLogger::get()->attachStream(
                new InviwoAssimpLogStream(LogLevel::Warn, filePath),
                Assimp::Logger::ErrorSeverity::Warn);
        }
        if (logLevel_ >= AssimpLogLevel::Info) {
            Assimp::DefaultLogger::get()->attachStream(
                new InviwoAssimpLogStream(LogLevel::Info, filePath),
                Assimp::Logger::ErrorSeverity::Info);
        }
        if (logLevel_ >= AssimpLogLevel::Debug) {
            Assimp::DefaultLogger::get()->attachStream(
                new InviwoAssimpLogStream(LogLevel::Info, filePath),
                Assimp::Logger::ErrorSeverity::Debugging);
        }
    }

    //#define AI_CONFIG_PP_SBP_REMOVE "aiPrimitiveType_POINTS | aiPrimitiveType_LINES"
    //#define AI_CONFIG_PP_FD_REMOVE 1

    unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
                         aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices |
                         aiProcess_ValidateDataStructure | aiProcess_ImproveCacheLocality |
                         aiProcess_RemoveRedundantMaterials | aiProcess_OptimizeMeshes;
    //      aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_TransformUVCoords |
    //      aiProcess_FindInstances
    //      aiProcess_OptimizeGraph | aiProcess_SortByPType | aiProcess_FindDegenerates |
    // aiProcess_OptimizeGraph is incompatible to aiProcess_PreTransformVertices

    if (fixInvalidData_) {
        flags |= aiProcess_FindInvalidData;
    }

    const aiScene* scene = importer.ReadFile(std::string(filePath), flags);

    std::clock_t start_convert = std::clock();
    if (logging) {
        Assimp::DefaultLogger::get()->debug(
            "time to load: " +
            std::to_string(double((std::clock() - start_readmetadata) / CLOCKS_PER_SEC)));
    }

    if (!scene) {
        throw DataReaderException(importer.GetErrorString(), IVW_CONTEXT);
    }

    // at least one mesh
    if (0 == scene->mNumMeshes) {
        throw DataReaderException("there are no meshes!", IVW_CONTEXT);
    }

    // because we use aiProcess_PreTransformVertices we can safely ignore the scenegraph,
    // nevertheless we must not ignore all the other differences:
    // to load multiple Assimp meshes (from one model) in one inviwo mesh we add a vertex counter to
    // correctly calculate the indices, assume all meshes have normal data or not (both options can
    // be fixed with preprocessing);
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
        if (logging) {
            Assimp::DefaultLogger::get()->debug("model has materials.");
        }
    }

    // create Inviwo's data structures for the model
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
    mesh->addBuffer(Mesh::BufferInfo(BufferType::PositionAttrib), pbuff);

    if (use_normals) {
        mesh->addBuffer(Mesh::BufferInfo(BufferType::NormalAttrib), nbuff);
    }

    // use additional unused attribute locations for extra color channels and texture coords
    int auxLocation = static_cast<int>(BufferType::Unknown);
    for (size_t i = 0; i < color_channels; ++i) {
        int location = (i == 0 ? static_cast<int>(BufferType::ColorAttrib) : auxLocation++);
        mesh->addBuffer(Mesh::BufferInfo(BufferType::ColorAttrib, location), cbuff[i]);
    }

    // texture coords
    for (size_t i = 0; i < texture_channels; ++i) {
        int location = (i == 0 ? static_cast<int>(BufferType::TexCoordAttrib) : auxLocation++);
        mesh->addBuffer(Mesh::BufferInfo(BufferType::TexCoordAttrib, location), tbuff[i]);
    }

    mesh->addIndices(Mesh::MeshInfo(dt, ConnectivityType::None), inds);

    std::clock_t now = std::clock();
    if (logging) {
        Assimp::DefaultLogger::get()->debug(
            "time to convert: " + std::to_string(double((now - start_convert) / CLOCKS_PER_SEC)));
        Assimp::DefaultLogger::get()->debug(
            "overall time to import: " +
            std::to_string(double((now - start_readmetadata) / CLOCKS_PER_SEC)));

        Assimp::DefaultLogger::kill();
    }

    return mesh;
}

bool AssimpReader::setOption(std::string_view key, std::any value) {
    if (auto* fix = std::any_cast<bool>(&value); fix && key == "FixInvalidData") {
        setFixInvalidDataFlag(*fix);
        return true;
    } else if (auto* level = std::any_cast<LogVerbosity>(&value); level && key == "LogLevel") {
        switch (*level) {
            case LogVerbosity::Error:
                setLogLevel(AssimpLogLevel::Error);
                return true;
            case LogVerbosity::Warn:
                setLogLevel(AssimpLogLevel::Warn);
                return true;
            case LogVerbosity::Info:
                setLogLevel(AssimpLogLevel::Info);
                return true;
            case LogVerbosity::None:
                setLogLevel(AssimpLogLevel::None);
                return true;
        }
        return false;
    }

    return false;
}

std::any AssimpReader::getOption(std::string_view key) {
    if (key == "FixInvalidData") {
        return getFixInvalidDataFlag();
    } else if (key == "LogLevel") {
        switch (getLogLevel()) {
            case AssimpLogLevel::Error:
                return LogVerbosity::Error;
            case AssimpLogLevel::Warn:
                return LogVerbosity::Warn;
            case AssimpLogLevel::Info:
                return LogVerbosity::Info;
            case AssimpLogLevel::Debug:
                return LogVerbosity::Info;
            case AssimpLogLevel::None:
                return LogVerbosity::None;
        }
    }
    return std::any{};
}

}  // namespace inviwo
