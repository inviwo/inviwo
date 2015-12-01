/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2015, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

// File format references:
// http://graphcomp.com/info/specs/sgi/vrml/spec/
// http://graphcomp.com/info/specs/vrml11.html
// http://www.graphcomp.com/info/specs/ibm/vrml_bin.html


/** @file
 * Implementation of the VRML (Version 1) importer class.
 *  Supports loading of .iv Open Inventor files as well.
 * Incomplete implementation!
 */

#include "AssimpPCH.h"

#ifndef ASSIMP_BUILD_NO_VRML1_IMPORTER

#include "VRML1Importer.h"
#include "VRMLUtil.h"

#include <iterator>

using namespace Assimp;
using namespace VRML;

// ---------------------------------------------------------------------

/** @brief Return importer meta information.
 * See #BaseImporter::GetInfo for the details
 */
const aiImporterDesc *VRML1Importer::GetInfo () const {
	return &VRMLImporter_descr;
}

// ---------------------------------------------------------------------

/** @brief Returns whether the class can handle the format of the given file.
 */
bool VRML1Importer::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
	const std::string extension = GetExtension(pFile);

	// dont check for .iv!
	// since its similar to VRML 1, we can read it but will
	// do so only if there is no better .iv importer.
	if (extension == "wrl") {
		if (pIOHandler) {
			const char* token_version[] = {"2."};
			return (false == SearchFileHeaderForToken(pIOHandler,pFile,token_version,1));
		}
	}

	if ((!extension.length() || checkSig) && pIOHandler) {
		// Is vrml in the Header line?
		const char* tokens[] = {"vrml"};
		if (false == SearchFileHeaderForToken(pIOHandler,pFile,tokens,1)) {
			// Open Inventor .iv?
			const char* tokens_iv[] = {"inventor"};
			return SearchFileHeaderForToken(pIOHandler,pFile,tokens_iv,1);
		} else {
			// Version 1 VRML?
			const char* token_version[] = {"2."};
			return (false == SearchFileHeaderForToken(pIOHandler,pFile,token_version,1));
		}
	}

	return false;
}

// ---------------------------------------------------------------------

/** @brief This function will be called to read a VRML 1 file
 */
void VRML1Importer::InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
	std::clock_t start_import_time = std::clock();
	// enforce logging in debug mode
	//#if (defined ASSIMP_BUILD_DEBUG)
	//	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDERR);
	//#endif

	const char* tokens[] = {"2."};
	bool VRML2 = SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);
	if (VRML2) {
		const char* tokens_iv[] = {"inventor"};
		VRML2 = !SearchFileHeaderForToken(pIOHandler,pFile,tokens_iv,1);
	}

	if (VRML2)
		ASSIMP_VRML_LOG_ERROR("Warning: cant load VRML2; this Importer is for Version 1!\n\
							Most propably this *will* fail/result in wrong geometry!\n");

	boost::scoped_ptr<IOStream> file(pIOHandler->Open(pFile, "rb"));

	if( file.get() == NULL)
		return ASSIMP_VRML_THROW_aiEXCEPTION("Failed to open file " + pFile + ".");

	size_t filesize = file->FileSize();

	std::vector<char> buffer;
	buffer.reserve(filesize);
	// ConvertToUTF8() is already called in TextFileToBuffer
	TextFileToBuffer(file.get(), buffer);

	std::vector<std::string> string_reference;
	std::stringstream sstream(preprocess(buffer, string_reference));
	// #if (defined ASSIMP_BUILD_DEBUG)
		// std::cout << "########################################################" << std::endl;
		// std::cout << sstream.str() << std::endl;
		// std::cout << "########################################################" << std::endl;
	// #endif

	ASSIMP_VRML_LOG_DEBUG("time to preprocess: " + TypToStr<double>(double((std::clock() - start_import_time)/ CLOCKS_PER_SEC)));

	{
		// static std::vector<VRML1_parsedata *> storage;
		// storage.push_back(new VRML1_parsedata());
		// VRML1_parsedata &pdata = *(storage.back());
		VRML1_parsedata pdata;
		std::string rtname = "VRML1_Root";
		{ // initialize the parsing data
			VRML1_state state;
			state.nodename = rtname;
			pdata.PushState(state);
			pdata.nodehierarchy.push(rtname);
			std::size_t ndind = 0;
			pdata.nodeindices.push(ndind);
			aiNode *nd = new aiNode();
			nd->mName = *(new aiString(rtname));
			pdata.nodes.push_back(nd);
		}

		std::string last = rtname;
		while (sstream) {
			std::string str;
			sstream >> str;

			if ("{" == str) {
				pdata.nodehierarchy.push(last);

				std::size_t nind = pdata.nodes.size();
				aiNode *nd = new aiNode;
				nd->mName = *(new aiString(last));
				nd->mParent = (aiNode *) pdata.nodeindices.last();
				pdata.nodes.at(pdata.nodeindices.last())->mNumChildren++;
				pdata.nodes.push_back(nd);
				pdata.nodeindices.push(nind);

				if ("Separator" == last) {
					VRML1_state state;
					state.nodename = last;
					pdata.PushState(state);
				} else {
					VRML1_parse(pdata, sstream, last);
				}
			} else if ("}" == str) {
				if ("Separator" == pdata.nodehierarchy.last())
					pdata.PopState();
				VRML1_end_node(pdata);
			}

			last = str;
		}

		if (rtname != pdata.nodehierarchy.last())
			return ASSIMP_VRML_THROW_aiEXCEPTION("failed to parse file " + pFile + " due to missmatching braces.");
		
		std::time_t start_fill_scene_data_time = std::time(NULL);
		VRML1_fill_scene_data(pScene, pdata);
		ASSIMP_VRML_LOG_DEBUG("time to fill scene data: " + TypToStr<double>(double((std::clock() - start_fill_scene_data_time) / CLOCKS_PER_SEC)));
	}

	// sstream.fail() is always true due to while (sstream)
	if (sstream.bad() || !sstream.eof()) {
		ASSIMP_VRML_LOG_ERROR("failed to parse file " + pFile + "completely (bad: " + TypToStr<bool>(sstream.bad()) +
			", fail: " + TypToStr<bool>(sstream.fail()) + ", eof: " + TypToStr<bool>(sstream.eof()) + ").");
	}

	ASSIMP_VRML_LOG_DEBUG("time to load: " + TypToStr<double>(double((std::clock() - start_import_time) / CLOCKS_PER_SEC)));
}


// *********************************************************************
/*
 * internal parse functionality
 */
// *********************************************************************

// ---------------------------------------------------------------------

// is called on }
void VRML1Importer::VRML1_end_node(VRML1_parsedata &pdata) {
	pdata.nodehierarchy.pop();
	pdata.nodeindices.pop();
}

// ---------------------------------------------------------------------

// VRML keywords are implemented here, faster with a std::map?
void VRML1Importer::VRML1_parse(VRML1_parsedata &pdata, std::stringstream &sstream, std::string last) {
	VRML1_state &state = pdata.StateStack.last();
	if ("Normal" == last) {
		std::string str;
		sstream >> str; // skip the vector keyword
		read_Floats(sstream, state.Normal);
	} else if ("Coordinate3" == last) {
		std::string str;
		sstream >> str; // skip the point keyword
		read_Floats(sstream, state.Coordinate);
	} else if ("Material" == last) {
		state.Material.VRMLParse(sstream);
		VRML1_end_node(pdata);
	} else if ("MaterialBinding" == last) {
		std::string str;
		sstream >> str; // skip the value keyword
		sstream >> str;
		state.MaterialBinding = VRMLParseBinding(str);
	} else if ("NormalBinding" == last) {
		std::string str;
		sstream >> str; // skip the value keyword
		sstream >> str;
		state.NormalBinding = VRMLParseBinding(str);
	} else if ("IndexedFaceSet" == last) {
		IndexedFaceSet fs = VRML1_parse_IndexedFaceSet(sstream);
		VRML1_constr_mesh_faceset(pdata, fs);
		VRML1_end_node(pdata);
	}
}

// ---------------------------------------------------------------------

IndexedFaceSet VRML1Importer::VRML1_parse_IndexedFaceSet(std::stringstream &sstream) {
	IndexedFaceSet fs;

	while (true) {
		std::string str;
		sstream >> str;

		if (!sstream)
			ASSIMP_VRML_THROW_aiEXCEPTION("fatal error parsing IndexedFaceSet!");

		if ("}" == str) {
			break;
		} else if ("coordIndex" == str) {
			read_Ints(sstream, fs.coordIndex);
		} else if ("materialIndex" == str) {
			read_Ints(sstream, fs.materialIndex);
		} else if ("normalIndex" == str) {
			read_Ints(sstream, fs.normalIndex);
		} else if ("textureCoordIndex" == str) {
			read_Ints(sstream, fs.textureCoordIndex);
		} else {
			ASSIMP_VRML_LOG_WARN(str + " is ignored inside IndexedFaceSet");
		}
	}

	if (fs.coordIndex.empty())
		ASSIMP_VRML_THROW_aiEXCEPTION("IndexedFaceSet must contain coordinate indices");

	return fs;
}

// ---------------------------------------------------------------------

void VRML1Importer::VRML1_constr_mesh_faceset(VRML1_parsedata &pdata, IndexedFaceSet fs) {
	VRML1_state st = pdata.StateStack.last();
	if (st.Coordinate.empty())
		ASSIMP_VRML_THROW_aiEXCEPTION("No coordinates available for IndexedFaceSet");

	// find out how the data needs to be accessed
	bool use_mat_indices =
		(BINDINGS_PER_PART_INDEXED == st.MaterialBinding) ||
		(BINDINGS_PER_FACE_INDEXED == st.MaterialBinding) ||
		(BINDINGS_PER_VERTEX_INDEXED == st.MaterialBinding);

	bool use_mat_per_vertex =
		(BINDINGS_PER_VERTEX == st.MaterialBinding) ||
		(BINDINGS_PER_VERTEX_INDEXED == st.MaterialBinding);

	bool use_only_one_material =
		(BINDINGS_OVERALL == st.MaterialBinding) ||
		(BINDINGS_DEFAULT == st.MaterialBinding);

	bool use_mat_per_face = (false == use_only_one_material) && (false == use_mat_per_vertex);

	bool use_normal_indices =
		(BINDINGS_PER_PART_INDEXED == st.NormalBinding) ||
		(BINDINGS_PER_FACE_INDEXED == st.NormalBinding) ||
		(BINDINGS_DEFAULT == st.NormalBinding) ||
		(BINDINGS_PER_VERTEX_INDEXED == st.NormalBinding);

	bool use_normal_per_vertex =
		(BINDINGS_PER_VERTEX == st.NormalBinding) ||
		(BINDINGS_DEFAULT == st.NormalBinding) ||
		(BINDINGS_PER_VERTEX_INDEXED == st.NormalBinding);

	bool use_normals = BINDINGS_OVERALL != st.NormalBinding;

	// bool use_normal_per_face = (!use_normal_per_vertex) && use_normals;


	// do some checks
	if ((false == use_normal_indices) && (false == fs.normalIndex.empty())) {
		ASSIMP_VRML_LOG_WARN("The normal indices inside the IndexedFaceSet are ignored because of the NormalBinding setting");
	}

	if ((false == use_mat_indices) && (false == fs.materialIndex.empty())) {
		ASSIMP_VRML_LOG_WARN("The material indices inside the IndexedFaceSet are ignored because of the MaterialBinding setting");
	}

	if (use_normal_indices && fs.normalIndex.empty()) {
		ASSIMP_VRML_LOG_ERROR("Expected Normal Indices because of the NormalBinding, but found none.");
		use_normals = false;
		use_normal_indices = false;
	}

	if (use_mat_indices && fs.materialIndex.empty()) {
		ASSIMP_VRML_LOG_ERROR("Expected Material Indices because of the MaterialBinding, but found none.");
		use_only_one_material = true;
		use_mat_indices = false;
	}

	if (use_mat_per_vertex) {
		ASSIMP_VRML_LOG_WARN("Material per Vertex is not supported. Material of the last vertex inside each face will be applied to the complete face.");
	}

	if (st.Normal.empty()) {
		if (use_normals)
			ASSIMP_VRML_LOG_WARN("Expected Normal Data because of the NormalBinding, but found none.");
		use_normals = false;
		// use_normal_per_face = false;
		use_normal_per_vertex = false;
		use_normal_indices = false;
	}


	// change the data layout to Assimp's
	std::map<unsigned int, VRML1_mesh> meshes;
	std::vector<VRML1_vertex> face_vertices;
	fs.coordIndex.push_back(-1);

	try {
		std::size_t count = fs.coordIndex.size();
		std::size_t i = 0;
		std::size_t face_index = 0;
		while (i<count) {
			if (0 > fs.coordIndex[i]) {
				if (!face_vertices.empty()) {
					unsigned int material_index = 0;
					if (false == use_only_one_material) {
						if (use_mat_per_face) {
							if (use_mat_indices) {
								material_index = (unsigned int) fs.materialIndex.at(face_index);
							} else {
								material_index = face_index;
							}
						} else if (use_mat_per_vertex) {
							if (use_mat_indices) {
								material_index = (unsigned int) fs.materialIndex.at(i-1);
							} else {
								material_index = (unsigned int) fs.coordIndex.at(i-1);
							}
						}
					}

					std::map<unsigned int, VRML1_mesh>::iterator it = meshes.find(material_index);
					if (meshes.end() == it) {
						meshes[material_index].vertices = face_vertices;
						meshes[material_index].faces.push_back(face_vertices.size());
					} else {
						(it->second).faces.push_back(face_vertices.size());
						std::copy(face_vertices.begin(), face_vertices.end(), std::back_inserter((it->second).vertices));
					}

					face_vertices.clear();
				}

				face_index++;
			} else {
				VRML1_vertex v;
				v.position = GetAiVector3D(st.Coordinate, fs.coordIndex.at(i));
				if (use_normals) {
					if (use_normal_per_vertex) {
						if (use_normal_indices) {
							v.normal = GetAiVector3D(st.Normal, fs.normalIndex.at(i));
						} else {
							v.normal = GetAiVector3D(st.Normal, fs.coordIndex.at(i));
						}
					} else {
						if (use_normal_indices) {
							v.normal = GetAiVector3D(st.Normal, fs.normalIndex.at(face_index));
						} else {
							v.normal = GetAiVector3D(st.Normal, face_index);
						}
					}
				}

				face_vertices.push_back(v);
			}

			i++;
		}
	} catch (std::out_of_range ex) {
		ASSIMP_VRML_THROW_aiEXCEPTION("invalid index data!");
	}


	// add the data to the assimp structures
	unsigned int mesh_start_index = pdata.meshes.size();
	std::map<unsigned int, VRML1_mesh>::iterator msit;
	for (msit = meshes.begin(); msit != meshes.end(); ++msit) {
		std::pair<unsigned int, VRML1_mesh> mit = *msit;

		pdata.meshes.push_back(new aiMesh());
		aiMesh &m = *(pdata.meshes.back());

		// Mesh
		VRML1_mesh mesh = mit.second;

		m.mNumFaces = mesh.faces.size();
		m.mFaces = new aiFace[mesh.faces.size()];
		m.mVertices = new aiVector3D[mesh.vertices.size()];
		m.mNumVertices = mesh.vertices.size();
		if (use_normals) {
			m.mNormals = new aiVector3D[mesh.vertices.size()];
		}

		// wont work, every index block must be allocated separate, see mesh.h
		// unsigned int *indexarr = new unsigned int[mesh.vertices.size()];

		for (size_t i = 0; i < mesh.vertices.size(); ++i) {
			m.mVertices[i] = mesh.vertices[i].position;
			if (use_normals) {
				m.mNormals[i] = mesh.vertices[i].normal;
			}

		//	indexarr[i] = i;
		}

		size_t pindex = 0;
		for (std::size_t i = 0; i < mesh.faces.size(); ++i) {
			m.mFaces[i].mNumIndices = mesh.faces[i];
			// m.mFaces[i].mIndices = &indexarr[pindex];
			m.mFaces[i].mIndices = new unsigned int[mesh.faces[i]];
			for (std::size_t j = 0; j < mesh.faces[i]; ++j) {
				(m.mFaces[i].mIndices)[j] = j + pindex;
			}

			pindex += mesh.faces[i];
		}

		// Material
		m.mMaterialIndex = pdata.materials.size();
		unsigned int mindex = mit.first;
		pdata.materials.push_back(st.Material.getAiMat(mindex));
	}

	aiNode &nd = *(pdata.nodes.at(pdata.nodeindices.last()));
	nd.mNumMeshes = meshes.size();
	nd.mMeshes = new unsigned int[nd.mNumMeshes];
	for (std::size_t i = 0; i < nd.mNumMeshes; i++) {
		nd.mMeshes[i] = mesh_start_index + i;
	}
}

// ---------------------------------------------------------------------

void VRML1Importer::VRML1_fill_scene_data(aiScene *pScene, VRML1_parsedata &pdata) {
	pScene->mMeshes = new aiMesh*[pdata.meshes.size()];
	pScene->mNumMeshes = pdata.meshes.size();
	for (std::size_t i = 0; i < pScene->mNumMeshes; i++) {
		(pScene->mMeshes)[i] = pdata.meshes[i];
	}

	pScene->mNumMaterials = pdata.materials.size();
	if (0 < pScene->mNumMaterials) {
		pScene->mMaterials = new aiMaterial*[pdata.materials.size()];
		for (std::size_t i = 0; i < pScene->mNumMaterials; i++) {
			(pScene->mMaterials)[i] = pdata.materials[i];
		}
	}

	for (std::size_t i = 0; i < pdata.nodes.size(); i++) {
		(*(pdata.nodes[i])).mChildren = new aiNode*[(*(pdata.nodes[i])).mNumChildren];

		for (std::size_t ci = 0; ci < (*(pdata.nodes[i])).mNumChildren; ci++) {
			(*(pdata.nodes[i])).mChildren[ci] = NULL;
		}

		if (0 != i) {
			(*(pdata.nodes[i])).mParent = pdata.nodes[(std::size_t) ((*(pdata.nodes[i])).mParent)];

			for (std::size_t pi = 0; pi < (*(pdata.nodes[i])).mParent->mNumChildren; pi++) {
				if (NULL == (*(pdata.nodes[i])).mParent->mChildren[pi]) {
					(*(pdata.nodes[i])).mParent->mChildren[pi] = pdata.nodes[i];
					pi = (*(pdata.nodes[i])).mParent->mNumChildren;
				}
			}
		}
	}

	pScene->mRootNode = pdata.nodes[0];
	// (pScene->mRootNode)->mParent = NULL;
}

// ---------------------------------------------------------------------

#endif // ASSIMP_BUILD_NO_VRML1_IMPORTER
