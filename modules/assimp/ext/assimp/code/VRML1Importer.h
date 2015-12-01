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

/** @file VRML1Importer.h
 *  Declaration of the VRML (Version 1) importer class.
 *  Supports loading of .iv Open Inventor files as well.
 *  Incomplete implementation!
 */

#ifndef AI_VRML1IMPORTER_H_INCLUDED
#define AI_VRML1IMPORTER_H_INCLUDED

#include "BaseImporter.h"
#include "VRMLUtil.h"

namespace Assimp {

	/** @brief Importer class for the VRML file format
	*/
	class VRML1Importer : public BaseImporter {

		public:

			VRML1Importer() {}
			~VRML1Importer() {}


			/** @brief Returns whether the class can handle the format of the given file.
			 * See BaseImporter::CanRead() for details.
			 */
			bool CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const;


			/** @brief Return importer meta information.
			 * See #BaseImporter::GetInfo for the details
			 */
			const aiImporterDesc* GetInfo () const;


			/** @brief Imports the given file into the given scene structure.
			* See BaseImporter::InternReadFile() for details
			*/
			void InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);

		private:
			static void VRML1_parse(VRML::VRML1_parsedata &pdata, std::stringstream &sstream, std::string last);
			static VRML::IndexedFaceSet VRML1_parse_IndexedFaceSet(std::stringstream &sstream);
			static void VRML1_constr_mesh_faceset(VRML::VRML1_parsedata &pdata, VRML::IndexedFaceSet fs);
			static void VRML1_end_node(VRML::VRML1_parsedata &pdata);
			static void VRML1_fill_scene_data(aiScene *pScene, VRML::VRML1_parsedata &pdata);
	};

} // namespace Assimp

#endif // AI_VRML1IMPORTER_H_INCLUDED
