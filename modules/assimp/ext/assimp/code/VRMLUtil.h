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

#ifndef AI_VRMLUTIL_H_INCLUDED
#define AI_VRMLUTIL_H_INCLUDED

// include if VRML1/2 Importer is build
#define DONT_INCLUDE_THIS_HEADER

#ifndef ASSIMP_BUILD_NO_VRML1_IMPORTER
	#undef DONT_INCLUDE_THIS_HEADER
#endif

#ifndef ASSIMP_BUILD_NO_VRML2_IMPORTER
	#undef DONT_INCLUDE_THIS_HEADER
#endif

#include "AssimpPCH.h"

#ifndef DONT_INCLUDE_THIS_HEADER

#include "ParsingUtils.h"
#include "fast_atof.h"

namespace Assimp
{

	namespace VRML
	{
		// *****************************************************************
		/*
		 * VRML 1 & 2:
		 */
		// *****************************************************************

		bool has_prefix(std::string prefix, std::string str);

		// importer meta information.
		static const aiImporterDesc VRMLImporter_descr = {
			"VRML (.wrl) Importer",
			"",
			"",
			"",
			aiImporterFlags_SupportTextFlavour,
			0,
			0,
			0,
			0,
			"wrl"
		};

		/** @brief represents VRML Material Node*/
		class VRML_material {
		private:
			std::vector<float> ambientColor;
			std::vector<float> diffuseColor;
			std::vector<float> specularColor;
			std::vector<float> emissiveColor;
			std::vector<float> shininess;
			std::vector<float> transparency;

			aiColor3D getAmbientColor(std::size_t index);
			aiColor3D getDiffuseColor(std::size_t index);
			aiColor3D getSpecularColor(std::size_t index);
			aiColor3D getEmissiveColor(std::size_t index);
			float getShininess(std::size_t index);
			float getTransparency(std::size_t index);

		public:
			/** @brief get Material property
			*
			*   VRML Materials may contain multiple material properties of one type,
			*   i.e. 3 ambient rgb colors - therefore the index is used
			*   if not enough properties are provided = a invalid index is used a default value is returned*/
			aiMaterial *getAiMat(std::size_t index);
			/** @brief parse a text VRML Materialnode
			*
			*   In VRML1 this is called from a Separator::material field conaining a Material Node
			*   In VRML2 normally the context is a Apperance Node containing a material field*/
			void VRMLParse(std::stringstream &sstream);
		};

		// Logging functionality (VRML is prepended)
		void LogWarn(std::string msg, const char *file, unsigned int line, const char *function);
		void LogError(std::string msg, const char *file, unsigned int line, const char *function);
		void LogInfo(std::string msg, const char *file, unsigned int line, const char *function);
		void LogDebug(std::string msg, const char *file, unsigned int line, const char *function);
		void throw_aiException(std::string msg, const char *file, unsigned int line, const char *function);

#ifndef __FILE__
#define __FILE__ "<__FILE__>"
#endif

#ifndef __func__
#define __func__ "<__func__>"
#endif

#ifndef __LINE__
#define __LINE__ 0
#endif

		// actually used are now these macros
#define ASSIMP_VRML_LOG_WARN(msg) LogWarn(msg, __FILE__, __LINE__, __func__)
#define ASSIMP_VRML_LOG_ERROR(msg) LogError(msg, __FILE__, __LINE__, __func__)
#define ASSIMP_VRML_LOG_INFO(msg) LogInfo(msg, __FILE__, __LINE__, __func__)
#define ASSIMP_VRML_LOG_DEBUG(msg) LogDebug(msg, __FILE__, __LINE__, __func__)
#define ASSIMP_VRML_THROW_aiEXCEPTION(msg) throw_aiException(msg, __FILE__, __LINE__, __func__)

		// creates a string from any type that supports << on stringstream
		template<class T>
		std::string TypToStr(T value) {
			std::stringstream sstream;
			sstream << value;
			return sstream.str();
		}

		// converts a string into a lower one
		void str_to_lower(std::string &str);

		/** @brief converts a string into an Integer, allowing hex representation as well (as needed by the VRML Spec) */
		bool StrToInt(std::string istr, int &out);
		/** @brief reads a field with floating point data
		*
		*   VRML SFFloat & MFFloat Nodes are read by this function, removing the [] parantheses if necessary */
		void read_Floats(std::stringstream &sstream, std::vector<float> &data);
		/** @brief reads a field with integer data using StrToInt */
		void read_Ints(std::stringstream &sstream, std::vector<int> &data);

		/** @brief reads a field with integer data using StrToInt */
		std::string preprocess(std::vector<char> &buffer,
			std::vector<std::string> &string_reference);

		// utility function to get an AiVector from a std::vector containing floats
		aiVector3D GetAiVector3D(std::vector<float> &vec, std::size_t pos);

		// *****************************************************************
		/*
		 * VRML 1:
		 */
		// *****************************************************************

		/** @brief VRML1 normal and color binding modes */
		enum VRML1_binding_type {
			BINDINGS_DEFAULT = 0,
			BINDINGS_OVERALL = 1,
			BINDINGS_PER_PART = 2,
			BINDINGS_PER_PART_INDEXED = 4,
			BINDINGS_PER_FACE = 8,
			BINDINGS_PER_FACE_INDEXED = 16,
			BINDINGS_PER_VERTEX = 32,
			BINDINGS_PER_VERTEX_INDEXED = 64
		};

		// just a stack, can be replaced by a normal std::vector if you have time
		template<class T>
		struct stack {
			std::vector<T> s;
			void push(T& obj) {
				s.push_back(obj);
			}

			bool empty(void) {
				return s.empty();
			}

			void pop(void) {
				if (!s.empty())
					s.pop_back();
			}

			T& last(void) {
				static T tmp;
				tmp = T();
				if (s.empty())
					return tmp;
				else
					return s.back();
			}
		};

		/** @brief Internal VRML1 parsing state, used by VRML1_parsedata */
		class VRML1_state {
		public:
			std::string nodename;
			std::vector<float> Coordinate;
			std::vector<float> Normal;
			VRML1_binding_type NormalBinding;
			VRML_material Material;
			VRML1_binding_type MaterialBinding;

			// (?) TODO: std::vector and std::string (and therefore VRML_Material) should be properly initialized
			VRML1_state() : NormalBinding(BINDINGS_DEFAULT), MaterialBinding(BINDINGS_DEFAULT) {}
		};

		/** @brief contains the whole parsing state */
		struct VRML1_parsedata {
			// references to all meshes, when finished parsing they are copied to aiScene
			// (use stdlib (std::vector) realloc functionality, only call new directly(*!) on the meshes itself)
			// * important: assimp uses the heap, delete will be called on each object,
			// therefore a std::vector<aiMesh> is not suitable
			std::vector<aiMesh*> meshes;
			// all materials, ...
			std::vector<aiMaterial*> materials;
			// all aiNodes
			std::vector<aiNode*> nodes;

			// current parsing state
			stack<VRML1_state> StateStack;
			// the scenegraph
			stack<std::string> nodehierarchy;
			// scenegraph - indices for the nodes
			stack<std::size_t> nodeindices;

			void PushState(VRML1_state &state) {
				StateStack.push(state);
			}

			void PopState(void) {
				StateStack.pop();
			}
		};

		/** @brief implementation of the IndexedFaceSet Node */
		struct IndexedFaceSet {
			std::vector<int> coordIndex;
			std::vector<int> materialIndex;
			std::vector<int> normalIndex;
			std::vector<int> textureCoordIndex;
		};

		/** @brief translates a string into a VRML binding */
		VRML1_binding_type VRMLParseBinding(std::string str);

		struct VRML1_vertex {
			aiVector3D position;
			aiVector3D normal;
			aiVector3D texcoord;
		};

		struct VRML1_mesh {
			std::vector<VRML1_vertex> vertices;
			std::vector<unsigned int> faces;
		};

		// *****************************************************************
		/*
		 * VRML 2:
		 */
		// *****************************************************************

		/** @brief mimicks a(ny) scenegraph entry */
		struct VRML2_sgentry {
			std::string nodename;
			std::size_t nodeindex;
		};

		// texture coordinates are read, but currently not further implemented (in the processing) and therefore not visible for the importer
		struct VRML2_vertex {
			aiVector3D position;
			aiVector3D normal;
			aiVector3D texcoord;
			aiVector3D color;
		};

		struct VRML2_mesh {
			std::vector<VRML2_vertex> vertices;
			std::vector<unsigned int> faces;
		};

		class VRML2_parsedata;
		/** @brief a Geometry Node (baseclass) just in case other fields than IndexedFaceSet needs to be implemented */
		class VRML2_GeometryNode {
		public:
			virtual void VRML2_constr_mesh(VRML2_parsedata &pdata) = 0;
			virtual ~VRML2_GeometryNode() {}
		};

		class VRML2_parsedata {
		public:
			// parsing storage:
			std::vector<aiMesh*> meshes;
			std::vector<aiMaterial*> materials;
			std::vector<aiNode*> nodes;

			// current parsing state:
			VRML2_GeometryNode *curr_gnode;
			VRML_material curr_material;
			std::vector<VRML2_sgentry> scenegraph;

			VRML2_parsedata() : curr_gnode(NULL) {}
		};

		/** @brief a IndexedFaceSet */
		class VRML2_IndexedFaceSet : public VRML2_GeometryNode {
		private:
			std::vector<int> coordIndex;
			std::vector<int> colorIndex;
			std::vector<int> normalIndex;
			std::vector<int> textureCoordIndex;

			std::vector<float> color;
			std::vector<float> coord;
			std::vector<float> normal;
			std::vector<float> texCoord;

			bool colorPerVertex;
			bool normalPerVertex;

		public:
			void clear(void) {
				colorPerVertex = true;
				normalPerVertex = true;
				coordIndex.clear();
				colorIndex.clear();
				normalIndex.clear();
				textureCoordIndex.clear();
				color.clear();
				coord.clear();
				normal.clear();
				texCoord.clear();
			}

			// parse a IndexedFaceSet from a textstream
			void VRML2_parse(std::stringstream &sstream);
			// and then construct a mesh in assimps structure from it
			virtual void VRML2_constr_mesh(VRML2_parsedata &pdata);

			~VRML2_IndexedFaceSet() {}
		};
	}
}

#endif

#endif
