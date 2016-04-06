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
 * Implementation of the VRML (Version 2) importer class.
 * Incomplete implementation!
 */

//#define INCLUDED_BY_VRML_IMPORTER

#include "AssimpPCH.h"

#ifndef ASSIMP_BUILD_NO_VRML2_IMPORTER

#include "VRML2Importer.h"
#include "VRMLUtil.h"

using namespace Assimp;
using namespace VRML;

// ---------------------------------------------------------------------

/** @brief Return importer meta information.
 * See #BaseImporter::GetInfo for the details
 */
const aiImporterDesc *VRML2Importer::GetInfo () const {
	return &VRMLImporter_descr;
}

// ---------------------------------------------------------------------

/** @brief Returns whether the class can handle the format of the given file.
 */
bool VRML2Importer::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const
{
	const std::string extension = GetExtension(pFile);

	if ((extension == "wrl") && (!checkSig)) {
		if (pIOHandler) {
			const char* token_vrml[] = { "vrml" };
			const char* token_version[] = { "2." };
			return SearchFileHeaderForToken(pIOHandler, pFile, token_vrml, 1)
				&& SearchFileHeaderForToken(pIOHandler, pFile, token_version, 1);
		}
	}

	if ((!extension.length() || checkSig) && pIOHandler) {
		// Is vrml in the Header line?
		const char* tokens[] = {"vrml"};
		if (SearchFileHeaderForToken(pIOHandler,pFile,tokens,1)) {
			// Version 2 VRML?
			const char* token_version[] = {"2."};
			return SearchFileHeaderForToken(pIOHandler,pFile,token_version,1);
		} else {
			// we have a compatibility mode for Open Inventor 2.1 files, so try to read it if there is no better importer
			const char* tokens_inventor[] = { "inventor" };
			const char* tokens_version[] = { "v2.1" };
			return SearchFileHeaderForToken(pIOHandler, pFile, tokens_inventor, 1)
				&& SearchFileHeaderForToken(pIOHandler, pFile, tokens_version, 1);
		}
	}

	return false;
}

// ---------------------------------------------------------------------

/** @brief This function will be called to read a VRML 2 file
 */
void VRML2Importer::InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler)
{
	std::clock_t start_import_time = std::clock();

	// enforce logging in debug mode
	//#if (defined ASSIMP_BUILD_DEBUG)
	//	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDERR);
	//#endif

	const char* tokens[] = {"2."};
	bool VRML2 = SearchFileHeaderForToken(pIOHandler,pFile,tokens,1);

	if (!VRML2)
		ASSIMP_VRML_LOG_ERROR("Warning: cant load VRML1; this Importer is for Version 2!\n\
							Most propably this *will* fail/result in wrong geometry!\n");

	const char* tokens_iv[] = { "inventor" };
	const char* tokens_21[] = { "v2.1" };
	bool is_openinventor21 = SearchFileHeaderForToken(pIOHandler, pFile, tokens_iv, 1)
		&& SearchFileHeaderForToken(pIOHandler, pFile, tokens_21, 1);
	if (is_openinventor21)
		ASSIMP_VRML_LOG_INFO("Loading file in OpenInvetor 2.1 compatibility mode.");

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

	ASSIMP_VRML_LOG_DEBUG("time to preprocess: " + TypToStr<double>(double((std::clock() - start_import_time) / CLOCKS_PER_SEC)));

	{
		std::string rtname = "VRML2_Root";
		VRML2_parsedata pdata;
		{ // initialize the parsing data
			VRML2_sgentry rnode;
			rnode.nodename = rtname;
			rnode.nodeindex = 0;
			pdata.scenegraph.push_back(rnode);
			aiNode *nd = new aiNode();
			nd->mName = *(new aiString(rtname));
			pdata.nodes.push_back(nd);
		}

		std::string last = rtname;
		while (sstream) {
			std::string str;
			sstream >> str;
			
			// open inventor 2.1 compatibility fix:
			if (is_openinventor21) {
				if ((str.length() > 4) && (VRML::has_prefix(std::string("VRML"), str))) {
					// simply remove the VRML prefix
					str = str.substr(4);
				}
			}

			if ("{" == str) {
				std::size_t pindex = pdata.scenegraph.back().nodeindex;

				long unsigned int nind = pdata.nodes.size();
				VRML2_sgentry sgnode;

				sgnode.nodename = last;
				sgnode.nodeindex = nind;
				pdata.scenegraph.push_back(sgnode);

				aiNode *nd = new aiNode;
				nd->mName = *(new aiString(last));
				nd->mParent = (aiNode *) pindex;
				pdata.nodes.at(pindex)->mNumChildren++;
				pdata.nodes.push_back(nd);

				VRML2_parse(pdata, sstream, last);
			} else if ("}" == str) {
				if (!pdata.scenegraph.empty()) {
					if (std::string("Shape") == pdata.scenegraph.back().nodename) {
						if (NULL != pdata.curr_gnode) {
							pdata.curr_gnode->VRML2_constr_mesh(pdata);
						}
						pdata.curr_gnode = NULL;
						pdata.curr_material = VRML_material();
					}
					pdata.scenegraph.pop_back();
				}
			}

			last = str;
		}

		if ((pdata.scenegraph.empty()) || (rtname != pdata.scenegraph.back().nodename))
			return ASSIMP_VRML_THROW_aiEXCEPTION("failed to parse file " + pFile + " due to missmatching braces.");

		std::clock_t start_fill_scene_data_time = std::clock();
		VRML2_fill_scene_data(pScene, pdata);
		ASSIMP_VRML_LOG_DEBUG("time to fill scene data: " + TypToStr<double>(double((std::clock() - start_fill_scene_data_time) / CLOCKS_PER_SEC)));
	}

	// sstream.fail() is always true due to while (sstream)
	if (sstream.bad() || !sstream.eof()) {
		ASSIMP_VRML_LOG_ERROR("failed to parse file " + pFile + "completely (bad: " + TypToStr<bool>(sstream.bad())
			+ ", fail: " + TypToStr<bool>(sstream.fail()) + ", eof: " + TypToStr<bool>(sstream.eof()) + ").");
	}

	ASSIMP_VRML_LOG_DEBUG("time to load: " + TypToStr<double>(double((std::clock() - start_import_time) / CLOCKS_PER_SEC)));
}

// *********************************************************************
/*
 * internal parse functionality
 */
// *********************************************************************

// ---------------------------------------------------------------------

void VRML2Importer::VRML2_parse(VRML2_parsedata &pdata, std::stringstream &sstream, std::string last) {
	if ("Material" == last) {
		pdata.curr_material.VRMLParse(sstream);
		if (!pdata.scenegraph.empty())
			pdata.scenegraph.pop_back();
	} else if ("IndexedFaceSet" == last) {
		static VRML2_IndexedFaceSet ifs;
		ifs.clear();
		ifs.VRML2_parse(sstream);
		pdata.curr_gnode = (VRML2_GeometryNode *) &ifs;
		if (!pdata.scenegraph.empty())
			pdata.scenegraph.pop_back();
	}
}

// ---------------------------------------------------------------------

void VRML2Importer::VRML2_fill_scene_data(aiScene *pScene, VRML2_parsedata &pdata) {
	if (pdata.meshes.empty())
		return ASSIMP_VRML_THROW_aiEXCEPTION("no meshes found!");

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
}

#endif // ASSIMP_BUILD_NO_VRML2_IMPORTER
