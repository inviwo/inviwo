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


// build if VRML1/2 Importer is build
#define DONT_BUILD_THIS_FILE

#ifndef ASSIMP_BUILD_NO_VRML1_IMPORTER
	#undef DONT_BUILD_THIS_FILE
#endif

#ifndef ASSIMP_BUILD_NO_VRML2_IMPORTER
	#undef DONT_BUILD_THIS_FILE
#endif


#include "AssimpPCH.h"


#ifndef DONT_BUILD_THIS_FILE

#include <iterator>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

#include "VRMLUtil.h"
#include "ParsingUtils.h"
#include "fast_atof.h"

namespace Assimp
{

namespace VRML
{

// *********************************************************************
/*
 * VRML 1 & 2:
 */
// *********************************************************************


// ---------------------------------------------------------------------
// logging functionality
void LogWarn(std::string msg, const char *file, unsigned int line, const char *function) {
	DefaultLogger::get()->warn("VRML: " + msg + "[" + file + ": " + TypToStr<unsigned int>(line) + "(" + function + ")]");
}

void LogError(std::string msg, const char *file, unsigned int line, const char *function) {
	DefaultLogger::get()->error("VRML: " + msg + "[" + file + ": " + TypToStr<unsigned int>(line) +"(" + function + ")]");
}

void LogInfo(std::string msg, const char *file, unsigned int line, const char *function) {
	DefaultLogger::get()->info("VRML: " + msg + "[" + file + ": " + TypToStr<unsigned int>(line) +"(" + function + ")]");
}

void LogDebug(std::string msg, const char *file, unsigned int line, const char *function) {
	DefaultLogger::get()->debug("VRML: " + msg + "[" + file + ": " + TypToStr<unsigned int>(line) +"(" + function + ")]");
}

void throw_aiException(std::string msg, const char *file, unsigned int line, const char *function) {
	throw DeadlyImportError("VRML: " + msg + "[" + file + ": " + TypToStr<unsigned int>(line) +"(" + function + ")]");
}
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// utility functionality
void str_to_lower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// check for a prefix
bool has_prefix(std::string prefix, std::string str) {
	if (prefix.length() > str.length())
		return false;
	return prefix.end() == (std::mismatch(prefix.begin(), prefix.end(), str.begin())).first;
}
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------
// parsing functionality
inline bool StrToInt(std::string istr, int &out) {
	const char *c = istr.c_str();
	bool inv = false;
	const char *pout = 0;

	// always at least null terminated
	if ('-' == c[0]) {
		c++;
		inv = true;
	}

	out = strtoul_cppstyle(c, &pout);
	if (inv)
		out = -out;

	return (pout == istr.c_str() + istr.length());
}

// read single float or float field, optionally inside []
void read_Floats(std::stringstream &sstream, std::vector<float> &data) {
	//static double clock_ticks_wasted = 0.0;
	//std::clock_t start = std::clock();
	if (!sstream)
		return ASSIMP_VRML_THROW_aiEXCEPTION("cant read expected float(s), sstream is in a bad state!");

	//data.reserve(32);

	while (true) {
		float f = 0;
		// TODO Performance
		sstream >> f;

		if (!sstream) {
			// cant parse the float, clear all errors
			sstream.clear();
			std::streamoff lastpos = sstream.tellg();
			std::string str;
			// read a string
			sstream >> str;
			if ("[" != str) {
				// not a [, we want to stop now
				if ("]" != str) {
					// its not a ], we've read to far - go back
					sstream.seekg(lastpos, sstream.beg);
				}

				break;
			}
		}
		else {
			// add the data!
			data.push_back(f);
		}
	}
	//clock_ticks_wasted = double(std::clock() - start);
	//ASSIMP_VRML_LOG_DEBUG("float import time: " + TypToStr<double>(clock_ticks_wasted / CLOCKS_PER_SEC));
}

// reads a single Int or a field, optionally inside []
void read_Ints(std::stringstream &sstream, std::vector<int> &data) {
	if (!sstream)
		return ASSIMP_VRML_THROW_aiEXCEPTION("cant read expected int(s), sstream is in a bad state!");

	//data.reserve(32);

	while (true) {
		int i = 0;
		std::string s;

		sstream >> s;

		if (!StrToInt(s, i)) {
			if ("[" != s) {
				// not a [, we want to stop now
				if ("]" != s) {
					std::streamoff lastpos = sstream.tellg();
					// its not a ], we've read to far - go back
					sstream.seekg(lastpos - s.length(), sstream.beg);
				}

				break;
			}
		}
		else {
			data.push_back(i);
		}
	}
}

bool str_to_bool(std::string str, bool &result) {
	str_to_lower(str);

	if (str == "true")
		result = true;
	else if (str == "false")
		result = false;
	else
		return false;

	return true;
}

// skip count "words" (text separated by space/newline/tabs)
void skip(std::stringstream &sstream, std::size_t count = 1) {
	std::string str;
	for (std::size_t i = 0; i < count; ++i)
		sstream >> str;
}
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------
// VRML material implementation

// create assimp's aiMaterial from our VRML material
aiMaterial *VRML_material::getAiMat(std::size_t index) {
	aiMaterial *mat = new aiMaterial();

	aiString matname = aiString("material_names_not_implemented");
	mat->AddProperty(&matname, AI_MATKEY_NAME);

	aiColor3D ac = getAmbientColor(index);
	if (AI_SUCCESS != mat->AddProperty(&ac, 1, AI_MATKEY_COLOR_AMBIENT))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_COLOR_AMBIENT can't be set set correctly");

	float shininess = getShininess(index);
	if (AI_SUCCESS != mat->AddProperty(&shininess, 1, AI_MATKEY_SHININESS))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_SHININESS can't be set set correctly");

	float transparency = getTransparency(index);
	if (AI_SUCCESS != mat->AddProperty(&transparency, 1, AI_MATKEY_COLOR_TRANSPARENT))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_COLOR_TRANSPARENT can't be set set correctly");

	aiColor3D dc = getDiffuseColor(index);
	if (AI_SUCCESS != mat->AddProperty(&dc, 1, AI_MATKEY_COLOR_DIFFUSE))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_COLOR_DIFFUSE can't be set set correctly");

	aiColor3D ec = getEmissiveColor(index);
	if (AI_SUCCESS != mat->AddProperty(&ec, 1, AI_MATKEY_COLOR_EMISSIVE))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_COLOR_EMISSIVE can't be set set correctly");

	aiColor3D sc = getSpecularColor(index);
	if (AI_SUCCESS != mat->AddProperty(&sc, 1, AI_MATKEY_COLOR_SPECULAR))
		ASSIMP_VRML_LOG_ERROR("Material Property AI_MATKEY_COLOR_SPECULAR can't be set set correctly");

	return mat;
}

// parse a material node
void VRML_material::VRMLParse(std::stringstream &sstream) {
	std::map<std::string, std::vector<float>*> table;
	table["ambientColor"] = &ambientColor;
	table["diffuseColor"] = &diffuseColor;
	table["specularColor"] = &specularColor;
	table["emissiveColor"] = &emissiveColor;
	table["shininess"] = &shininess;
	table["transparency"] = &transparency;

	std::map<std::string, std::vector<float>*>::iterator it;
	std::string str;
	while (true) {
		sstream >> str;
		it = table.find(str);
		if (table.end() != it) {
			// read the field
			std::vector<float> *vptr = (it->second);
			read_Floats(sstream, *vptr);
		} else {
			if ("}" == str) {
				break;
			} else {
				// VRML2 ambientIntensity?
				if (str == std::string("ambientIntensity")) {
					std::vector<float> v;
					read_Floats(sstream, v);
					for (std::size_t i = 0; i < v.size(); ++i) {
						ambientColor.push_back(v[i]);
						ambientColor.push_back(v[i]);
						ambientColor.push_back(v[i]);
					}
				} else {
					// error / invalid field name
					if (str.empty() || (!sstream))
						return ASSIMP_VRML_THROW_aiEXCEPTION("unexpected parse error in Node Material.");
					ASSIMP_VRML_LOG_WARN("Invalid data? " + str + " is ignored inside Material Nodes.");
				}
			}
		}
	}
}

// getters - to keep getAiMat readable
aiColor3D VRML_material::getAmbientColor(std::size_t index) {
	index *= 3;
	if (ambientColor.size() <= 2 + index)
		return aiColor3D(0.2f);
	return aiColor3D(ambientColor[index+0], ambientColor[index+1], ambientColor[index+2]);
}

aiColor3D VRML_material::getDiffuseColor(std::size_t index) {
	index *= 3;
	if (diffuseColor.size() <= 2 + index)
		return aiColor3D(0.8f);
	return aiColor3D(diffuseColor[index+0], diffuseColor[index+1], diffuseColor[index+2]);
}

aiColor3D VRML_material::getSpecularColor(std::size_t index) {
	index *= 3;
	if (specularColor.size() <= 2 + index)
		return aiColor3D(0.0);
	return aiColor3D(specularColor[index+0], specularColor[index+1], specularColor[index+2]);
}

aiColor3D VRML_material::getEmissiveColor(std::size_t index) {
	index *= 3;
	if (emissiveColor.size() <= 2 + index)
		return aiColor3D(0.0);
	return aiColor3D(emissiveColor[index+0], emissiveColor[index+1], emissiveColor[index+2]);
}

float VRML_material::getShininess(std::size_t index) {
	if (shininess.size() <= index)
		return 0.2f;
	return shininess[index];
}

float VRML_material::getTransparency(std::size_t index) {
	if (transparency.size() <= index)
		return 0.0f;
		// return 0.2f;
	return transparency[index];
}
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------
// preprocessing step:
// remove comments, check utf8, shrink whitespaces to one single space (' ')
// => returns a easy to parse string for further processing
//
// Warning:
// UTF-8 needs further testing, use with caution:
// == non ascii chars only inside comments
// (ascii as a subset of utf-8 is usually enough for the geometry data)
// see void BaseImporter::ConvertToUTF8(std::vector<char>& data)
//
// param info:
// buffer - input char buffer
// string_reference - strings are moved into this reference (~ utf-8)
std::string preprocess(std::vector<char> &buffer, std::vector<std::string> &string_reference)
{
	// string to hold the preprocessed result
	std::string content;
	// speed it up for big files at the risk of wasting memory
	content.reserve(buffer.size());

	// true if preprocess position is inside a comment (from # to end of line)
	bool comment = false;
	// to strip out multiple spaces, true if theres already one at the end of content
	// (spaces in VRML are , \t\n - replace them with a simple " ")
	bool lastchar_isspace = false;
	// strings are substituted to string_<num> and placed in string_reference
	bool inside_string = false;
	// counter for the <num>
	size_t string_count = 0;
	// temporary container for the string substitution
	std::string tmpstr;
	// ignore utf-8 outside comments; warn if bytes are skipped
	bool warn_skipped_utf8_outside_comments = false;

	// loop through the whole file
	for (size_t i = 0; i < (size_t) buffer.size(); i++) {
		// if false the character is copied to the output
		bool discard = false;

		if (comment) {
			// skip comments until end of line
			if (IsLineEnd(buffer[i]))
				comment = false;
			discard = true;
		}
		else // string or normal data?
		{
			if (inside_string) {
				// end of the string?
				if ('"' == buffer[i]) {
					// \ in front of " escapes the quotes in strings
					if ((0 == i) || ('\\' != buffer[i-1])) {
						// we are done, add the string to our reference
						inside_string = false;
						string_reference.push_back(tmpstr);
						tmpstr.clear();
						string_count++;
						discard = true;
					} else {
						// we are not done
						inside_string = true;
					}
				}

				// if we are inside a string (between the quotes)
				if (inside_string) {
					// remember the character and discard the output, we'll use a index later
					tmpstr += buffer[i];
					discard = true;
				}
			}
			else // normal data
			{
				// UTF-8 is not handled outside comments/strings - skip these bytes
				unsigned char c = (unsigned char) buffer[i];
				if (c <= 127) {
					// the normal "ASCII-subset" case
					if (IsSpaceOrNewLine(buffer[i]) || ',' == buffer[i]) {
						// convert all continuous spaces/newlines to one ' ' (, is a *space* in VRML too)
						if (lastchar_isspace)
							discard = true;
						else {
							lastchar_isspace = true;
							buffer[i] = ' ';
						}
					}
					else // normal character
					{
						switch (buffer[i]) {
						case '#':
							// a comment starts
							comment = true;
							lastchar_isspace = false;
						case ']': // skip [ and ], so we can ignore it later
						case '[':
							discard = true;
							break;
						case '"':
							// a string starts
							inside_string = true;
							discard = true;
							// add our index to the stream
							content += std::string("string_") + TypToStr<size_t>(string_count);
						default:
							lastchar_isspace = false;
							break;
						}
					}
				} else {
					// TODO: better solution...
					warn_skipped_utf8_outside_comments = true;
					i++;
					discard = true;
				}
			}
		}

		if (!discard)
			content += buffer[i];
	}

	if (warn_skipped_utf8_outside_comments)
		ASSIMP_VRML_LOG_WARN("VRML: skipped some bytes - utf-8 characters outside comments!");

	return content + " ";
}
// ---------------------------------------------------------------------


// ---------------------------------------------------------------------
// remaining stuff...
aiVector3D GetAiVector3D(std::vector<float> &vec, std::size_t pos) {
	return aiVector3D(vec.at(0+pos*3), vec.at(1+pos*3), vec.at(2+pos*3));
}
// ---------------------------------------------------------------------


// *********************************************************************
/*
 * VRML 1:
 */
// *********************************************************************


// ---------------------------------------------------------------------
// material/normal binding parse utility

// define a x macro list with the types
#define STRING_TRANSLATION_TABLE_BINDINGS \
	X( "DEFAULT", BINDINGS_DEFAULT ) \
	X( "OVERALL", BINDINGS_OVERALL ) \
	X( "PER_PART", BINDINGS_PER_PART ) \
	X( "PER_PART_INDEXED", BINDINGS_PER_PART_INDEXED ) \
	X( "PER_FACE", BINDINGS_PER_FACE ) \
	X( "PER_FACE_INDEXED", BINDINGS_PER_FACE_INDEXED ) \
	X( "PER_VERTEX", BINDINGS_PER_VERTEX ) \
	X( "PER_VERTEX_INDEXED", BINDINGS_PER_VERTEX_INDEXED )

// fill the entries into a std::map
#define FILL_MAP(map, entry_key, entry_value ) map [ entry_key ] = entry_value;

// translate a string to the right binding type
VRML1_binding_type VRMLParseBinding(std::string str) {
	// translation dictionary
	static std::map<std::string, VRML1_binding_type> table;
	// fill it at the first usage:
	if (table.empty()) {
		#define X(a, b) FILL_MAP( table, a, b )
			STRING_TRANSLATION_TABLE_BINDINGS
		#undef X
	}

	// look up the binding
	std::map<std::string, VRML1_binding_type>::iterator it = table.find(str);
	if (table.end() != it)
		return it->second;

	ASSIMP_VRML_LOG_ERROR("Invalid resource binding type: " + str + ". Default binding will be used.");
	return BINDINGS_DEFAULT;
}

#undef STRING_TRANSLATION_TABLE_BINDINGS
#undef FILL_MAP
// ---------------------------------------------------------------------

// *********************************************************************
/*
 * VRML 2:
 */
// *********************************************************************

// ---------------------------------------------------------------------
/** @Brief parse a IndexedFaceSet with all its fields*/
void VRML2_IndexedFaceSet::VRML2_parse(std::stringstream &sstream) {
	while (true) {
		std::string str;
		sstream >> str;

		if (!sstream)
			ASSIMP_VRML_THROW_aiEXCEPTION("fatal error parsing IndexedFaceSet!");

		if ("}" == str) {
			break;
		} else if ("coordIndex" == str) {
			read_Ints(sstream, coordIndex);
		} else if ("colorIndex" == str) {
			read_Ints(sstream, colorIndex);
		} else if ("normalIndex" == str) {
			read_Ints(sstream, normalIndex);
		} else if ("textureCoordIndex" == str) {
			read_Ints(sstream, textureCoordIndex);
		} else if ("colorPerVertex" == str) {
			sstream >> str;
			if (false == str_to_bool(str, colorPerVertex))
				ASSIMP_VRML_LOG_ERROR("failed to parse colorPerVertex boolean " + str + ".");
		} else if ("normalPerVertex" == str) {
			sstream >> str;
			if (false == str_to_bool(str, normalPerVertex))
				ASSIMP_VRML_LOG_ERROR("failed to parse normalPerVertex boolean " + str + ".");
		} else if ("color" == str) {
			skip(sstream, 3);
			read_Floats(sstream, color);
			skip(sstream);
		} else if ("coord" == str) {
			skip(sstream, 3);
			read_Floats(sstream, coord);
			skip(sstream);
		} else if ("normal" == str) {
			skip(sstream, 3);
			read_Floats(sstream, normal);
			skip(sstream);
		} else if ("texCoord" == str) {
			skip(sstream, 3);
			read_Floats(sstream, texCoord);
			skip(sstream);
		} else {
			ASSIMP_VRML_LOG_WARN("Invalid Data? " + str + " is ignored inside IndexedFaceSet");
		}
	}
}

void VRML2_IndexedFaceSet::VRML2_constr_mesh(VRML2_parsedata &pdata) {
	if (coord.empty())
		ASSIMP_VRML_THROW_aiEXCEPTION("IndexedFaceSet must contain coordinates");

	if (coordIndex.empty())
		ASSIMP_VRML_THROW_aiEXCEPTION("IndexedFaceSet must contain coordinate indices");

	bool use_normals = !normal.empty();
	bool use_colors = !color.empty();

	bool use_normal_indices = !normalIndex.empty();
	bool use_color_indices = !colorIndex.empty();

	// do some checks

	if ((false == use_normals)&&(use_normal_indices)) {
		ASSIMP_VRML_LOG_ERROR("Normal data is missing, but normal indices are present!");
		use_normal_indices = false;
	}

	if ((false == use_colors)&&(use_color_indices)) {
		ASSIMP_VRML_LOG_ERROR("Color data is missing, but color indices are present!");
		use_color_indices = false;
	}

	// change the data layout to Assimp's
	VRML2_mesh mesh;
	size_t face_vertex_count = 0;
	coordIndex.push_back(-1);

	//std::clock_t start_convert_data_layout = std::clock();

	try {
		std::size_t count = coordIndex.size();
		std::size_t i = 0;
		std::size_t face_index = 0;
		while (i<count) {
			if (0 > coordIndex[i]) {
				if (0 < face_vertex_count) {
					mesh.faces.push_back(face_vertex_count);
					face_vertex_count=0;
				}

				face_index++;
			} else {
				VRML2_vertex v;
				v.position = GetAiVector3D(coord, coordIndex.at(i));
				if (use_normals) {
					if (normalPerVertex) {
						if (use_normal_indices) {
							v.normal = GetAiVector3D(normal, normalIndex.at(i));
						} else {
							v.normal = GetAiVector3D(normal, coordIndex.at(i));
						}
					} else {
						if (use_normal_indices) {
							v.normal = GetAiVector3D(normal, normalIndex.at(face_index));
						} else {
							v.normal = GetAiVector3D(normal, face_index);
						}
					}
				}

				if (use_colors) {
					if (colorPerVertex) {
						if (use_color_indices) {
							v.color = GetAiVector3D(color, colorIndex.at(i));
						} else {
							v.color = GetAiVector3D(color, coordIndex.at(i));
						}
					} else {
						if (use_color_indices) {
							v.color = GetAiVector3D(color, colorIndex.at(face_index));
						} else {
							v.color = GetAiVector3D(color, face_index);
						}
					}
				}

				mesh.vertices.push_back(v);
				face_vertex_count++;
			}

			i++;
		}
	} catch (std::out_of_range ex) {
		ASSIMP_VRML_THROW_aiEXCEPTION("invalid index data!");
	}
	//ASSIMP_VRML_LOG_DEBUG("CFS: time to add data to ifs: " + TypToStr<double>(double((std::clock() - start_convert_data_layout) / CLOCKS_PER_SEC)));
	
	//std::clock_t start_add_data = std::clock();
	// add the data to assimp's structures
	unsigned int mesh_index = pdata.meshes.size();
	pdata.meshes.push_back(new aiMesh());
	aiMesh &m = *(pdata.meshes.back());

	m.mNumFaces = mesh.faces.size();
	m.mFaces = new aiFace[mesh.faces.size()];
	m.mVertices = new aiVector3D[mesh.vertices.size()];
	m.mNumVertices = mesh.vertices.size();
	if (use_normals) {
		m.mNormals = new aiVector3D[mesh.vertices.size()];
	}
	if (use_colors) {
		m.mColors[0] = new aiColor4D[mesh.vertices.size()];
	}

	for (size_t i = 0; i < mesh.vertices.size(); ++i) {
		m.mVertices[i] = mesh.vertices[i].position;
		if (use_normals) {
			m.mNormals[i] = mesh.vertices[i].normal;
		}
		if (use_colors) {
			aiVector3D col = mesh.vertices[i].color;
			(m.mColors[0])[i] = aiColor4D(col.x, col.y, col.z, 1.0);
		}
	}
	//ASSIMP_VRML_LOG_DEBUG("CFS: time to add data to ifs: " + TypToStr<double>(double((std::clock() - start_add_data) / CLOCKS_PER_SEC)));

	//std::clock_t start_index_alloc = std::clock();
	size_t pindex = 0;
	for (std::size_t i = 0; i < mesh.faces.size(); ++i) {
		m.mFaces[i].mNumIndices = mesh.faces[i];
		// TODO Performance
		m.mFaces[i].mIndices = new unsigned int[mesh.faces[i]];
		for (std::size_t j = 0; j < mesh.faces[i]; ++j) {
			(m.mFaces[i].mIndices)[j] = j + pindex;
		}

		pindex += mesh.faces[i];
	}
	//ASSIMP_VRML_LOG_DEBUG("CFS: time to allocate indices: " + TypToStr<double>(double((std::clock() - start_index_alloc) / CLOCKS_PER_SEC)));

	// Material
	m.mMaterialIndex = pdata.materials.size();
	pdata.materials.push_back(pdata.curr_material.getAiMat(0));


	aiNode &nd = *(pdata.nodes.at(pdata.scenegraph.back().nodeindex));
	nd.mNumMeshes = 1;
	nd.mMeshes = new unsigned int[1];
	nd.mMeshes[0] = mesh_index;
}
// ---------------------------------------------------------------------

} // namespace VRML

} // namespace Assimp

#endif
