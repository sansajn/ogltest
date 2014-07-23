// TODO: ork mesh loader (copy paste from ork)
#include <memory>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert>
#include "mesh_loader.h"
#include "core/ptr.h"
#include "render/meshbuffers.h"

using namespace std;

static ptr<mesh_buffers> load(char * data, int size);

ptr<mesh_buffers> mesh_loader::load(std::string const & fname)
{
	ifstream fin(fname.c_str());  // TODO: zdielaj, kedze vytvorenie je drahe
	if (!fin.is_open())
		throw exception();  // TODO: specify exception

	fin.seekg(0, ios::end);
	istream::pos_type size = fin.tellg();
	fin.seekg(0);

	std::unique_ptr<char> buf(new char[size]);
	fin.read(buf.get(), size);

	if (fin.gcount() != size)
		throw exception();  // reding failed (not all character readed)

	fin.close();

	return ::load(buf.get(), size);
}


struct dummy_bounds
{
	float xmin, xmax, ymin, ymax, zmin, zmax;
};


ptr<mesh_buffers> load(char * data, int size)
{
	ptr<mesh_buffers> m = make_ptr<mesh_buffers>();

	char buf[256];
	istringstream in(string((char*) data, size));
//	e = e == NULL ? desc->descriptor : e;

	dummy_bounds bounds;

	try {
		in >> bounds.xmin;
		in >> bounds.xmax;
		in >> bounds.ymin;
		in >> bounds.ymax;
		in >> bounds.zmin;
		in >> bounds.zmax;

		in >> buf;

		if (strcmp(buf, "points") == 0) {
//			mode = POINTS;
			m->mode = GL_POINTS;
		} else if (strcmp(buf, "lines") == 0) {
//			mode = LINES;
			m->mode = GL_LINES;
		} else if (strcmp(buf, "linesadjacency") == 0) {
//			mode = LINES_ADJACENCY;
			m->mode = GL_LINES_ADJACENCY;
		} else if (strcmp(buf, "linestrip") == 0) {
//			mode = LINE_STRIP;
			m->mode = GL_LINE_STRIP;
		} else if (strcmp(buf, "linestripadjacency") == 0) {
//			mode = LINE_STRIP_ADJACENCY;
			m->mode = GL_LINE_STRIP_ADJACENCY;
		} else if (strcmp(buf, "triangles") == 0) {
//			mode = TRIANGLES;
			m->mode = GL_TRIANGLES;
		} else if (strcmp(buf, "trianglesadjacency") == 0) {
//			mode = TRIANGLES_ADJACENCY;
			m->mode = GL_TRIANGLES_ADJACENCY;
		} else if (strcmp(buf, "trianglestrip") == 0) {
//			mode = TRIANGLE_STRIP;
			m->mode = GL_TRIANGLE_STRIP;
		} else if (strcmp(buf, "trianglestripadjacency") == 0) {
//			mode = TRIANGLE_STRIP_ADJACENCY;
			m->mode = GL_TRIANGLE_STRIP_ADJACENCY;
		} else if (strcmp(buf, "trianglefan") == 0) {
//			mode = TRIANGLE_FAN;
			m->mode = GL_TRIANGLE_FAN;
		} else {
//			if (Logger::ERROR_LOGGER != NULL) {
//				log(Logger::ERROR_LOGGER, desc, e, "Invalid mesh topology '" + string(buf) + "'");
//			}
			throw exception();
		}

		unsigned int attributeCount;
		in >> attributeCount;

		int vertexSize = 0;
		int* attributeIds = new int[attributeCount];
		unsigned int *attributeComponents = new unsigned int[attributeCount];
//		AttributeType *attributeTypes = new AttributeType[attributeCount];
		GLenum *attributeTypes = new GLenum[attributeCount];
		bool *attributeNorms = new bool[attributeCount];

		try {
			for (unsigned int i = 0; i < attributeCount; ++i) {
				in >> attributeIds[i];
				in >> attributeComponents[i];

				in >> buf;
				if (strcmp(buf, "byte") == 0) {
//					attributeTypes[i] = A8I;
					attributeTypes[i] = GL_BYTE;
					vertexSize += attributeComponents[i] * 1;
				} else if (strcmp(buf, "ubyte") == 0) {
//					attributeTypes[i] = A8UI;
					attributeTypes[i] = GL_UNSIGNED_BYTE;
					vertexSize += attributeComponents[i] * 1;
				} else if (strcmp(buf, "short") == 0) {
//					attributeTypes[i] = A16I;
					attributeTypes[i] = GL_SHORT;
					vertexSize += attributeComponents[i] * 2;
				} else if (strcmp(buf, "ushort") == 0) {
//					attributeTypes[i] = A16UI;
					attributeTypes[i] = GL_UNSIGNED_SHORT;
					vertexSize += attributeComponents[i] * 2;
				} else if (strcmp(buf, "int") == 0) {
//					attributeTypes[i] = A32I;
					attributeTypes[i] = GL_INT;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "uint") == 0) {
//					attributeTypes[i] = A32UI;
					attributeTypes[i] = GL_UNSIGNED_INT;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "float") == 0) {
//					attributeTypes[i] = A32F;
					attributeTypes[i] = GL_FLOAT;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "double") == 0) {
//					attributeTypes[i] = A64F;
					attributeTypes[i] = GL_DOUBLE;
					vertexSize += attributeComponents[i] * 8;
				} else {
//					if (Logger::ERROR_LOGGER != NULL) {
//						log(Logger::ERROR_LOGGER, desc, e, "Invalid mesh vertex component type '" + string(buf) + "'");
//					}
					throw exception();
				}

				in >> buf;
				if (strcmp(buf, "true") == 0) {
					attributeNorms[i] = true;
				} else if (strcmp(buf, "false") == 0) {
					attributeNorms[i] = false;
				} else {
//					if (Logger::ERROR_LOGGER != NULL) {
//						log(Logger::ERROR_LOGGER, desc, e, "Invalid mesh vertex normalization '" + string(buf) + "'");
//					}
					throw exception();
				}
			}
		} catch (...) {
			delete[] attributeIds;
			delete[] attributeComponents;
			delete[] attributeTypes;
			delete[] attributeNorms;
			throw exception();
		}

		for (unsigned int i = 0; i < attributeCount; ++i) {
//			addAttributeBuffer(attributeIds[i], attributeComponents[i],
//									 vertexSize, attributeTypes[i], attributeNorms[i]);
			m->append_attribute(attributeIds[i], attributeComponents[i],
				vertexSize, attributeTypes[i], attributeNorms[i]);
		}
		delete[] attributeIds;
		delete[] attributeComponents;
		delete[] attributeTypes;
		delete[] attributeNorms;

		unsigned int vertexCount;
		in >> vertexCount;
//		nvertices = vertexCount;
		m->nvertices = vertexCount;

		unsigned char* vertexBuffer = new unsigned char[vertexCount * vertexSize];
		unsigned int offset = 0;

		for (unsigned int i = 0; i < vertexCount; ++i) {
			for (unsigned int j = 0; j < attributeCount; ++j) {
//				ptr<AttributeBuffer> ab = getAttributeBuffer(j);
				ptr<attribute_buffer> ab = m->attribute(j);
//				for (int k = 0; k < ab->getSize(); ++k) {
				for (int k = 0; k < ab->size(); ++k) {
//					switch (ab->getType()) {
					switch (ab->type()) {
//						case A8I: {
						case GL_BYTE: {
							int ic;
							in >> ic;
							char c = (char) ic;
							memcpy(vertexBuffer + offset, &c, sizeof(char));
							offset += sizeof(char);
							break;
						}
//						case A8UI: {
						case GL_UNSIGNED_BYTE: {
							int iuc;
							in >> iuc;
							unsigned char uc = (unsigned char) iuc;
							memcpy(vertexBuffer + offset, &uc, sizeof(unsigned char));
							offset += sizeof(unsigned char);
							break;
						}
//						case A16I: {
						case GL_SHORT: {
							short s;
							in >> s;
							memcpy(vertexBuffer + offset, &s, sizeof(short));
							offset += sizeof(short);
							break;
						}
//						case A16UI: {
						case GL_UNSIGNED_SHORT: {
							unsigned short us;
							in >> us;
							memcpy(vertexBuffer + offset, &us, sizeof(unsigned short));
							offset += sizeof(unsigned short);
							break;
						}
//						case A32I: {
						case GL_INT: {
							int si;
							in >> si;
							memcpy(vertexBuffer + offset, &si, sizeof(int));
							offset += sizeof(int);
							break;
						}
//						case A32UI: {
						case GL_UNSIGNED_INT: {
							unsigned int ui;
							in >> ui;
							memcpy(vertexBuffer + offset, &ui, sizeof(unsigned int));
							offset += sizeof(unsigned int);
							break;
						}
						// TODO: not implemented
//						case A16F: {
//							half h;
//							float f;
//							in >> f;
//							h = f;
//							memcpy(vertexBuffer + offset, &h, sizeof(half));
//							offset += sizeof(half);
//							break;
//						}
//						case A32F: {
						case GL_FLOAT: {
							float f;
							in >> f;
							memcpy(vertexBuffer + offset, &f, sizeof(float));
							offset += sizeof(float);
							break;
						}
//						case A64F: {
						case GL_DOUBLE: {
							double d;
							in >> d;
							memcpy(vertexBuffer + offset, &d, sizeof(double));
							offset += sizeof(double);
							break;
						}

							// not handled (don't know why)
//						case A32I_2_10_10_10_REV:
//						case A32UI_2_10_10_10_REV:
//						case A32I_FIXED:
						default:
						{
							assert(false); // unsupported
							break;
						}
					}
				}
			}
		}

//		ptr<GPUBuffer> gpub = new GPUBuffer();
		ptr<gpubuffer> gpub(new gpubuffer());
//		gpub->setData(vertexCount * vertexSize, vertexBuffer, STATIC_DRAW);
		gpub->data(vertexCount * vertexSize, vertexBuffer, GL_STATIC_DRAW);
//		for (int i = 0; i < getAttributeCount(); ++i) {
		for (int i = 0; i < m->attribute_count(); ++i) {
//			getAttributeBuffer(i)->setBuffer(gpub);
			m->attribute(i)->buf(gpub);
		}
		delete[] vertexBuffer;

		unsigned int indiceCount;
		in >> indiceCount;
//		nindices = indiceCount;
		unsigned int nindices = indiceCount;
		m->nindices = nindices;

		if (nindices > 0) {
			int indiceSize;
//			AttributeType type;
			GLenum type;
			if (vertexCount < 256) {
				indiceSize = 1;
//				type = A8UI;
				type = GL_UNSIGNED_BYTE;
			} else if (vertexCount < 65536) {
				indiceSize = 2;
//				type = A16UI;
				type = GL_UNSIGNED_SHORT;
			} else {
				indiceSize = 4;
//				type = A32UI;
				type = GL_UNSIGNED_INT;
			}

			unsigned char* indiceBuffer = new unsigned char[indiceCount * indiceSize];
			offset = 0;

			if (indiceSize == 1) {
				for (unsigned int i = 0; i < indiceCount; ++i) {
					int ic;
					in >> ic;
					unsigned char c = (unsigned char) ic;
					memcpy(indiceBuffer + offset, &c, 1);
					offset += 1;
				}
			} else if (indiceSize == 2) {
				for (unsigned int i = 0; i < indiceCount; ++i) {
					int ic;
					in >> ic;
					unsigned short c = (unsigned short) ic;
					memcpy(indiceBuffer + offset, &c, 2);
					offset += 2;
				}
			} else {
				for (unsigned int i = 0; i < indiceCount; ++i) {
					int ic;
					in >> ic;
					char c = (char) ic;
					memcpy(indiceBuffer + offset, &c, indiceSize);
					offset += indiceSize;
				}
			}

//			gpub = new GPUBuffer();
			gpub = make_ptr<gpubuffer>();
//			gpub->setData(indiceCount * indiceSize, indiceBuffer, STATIC_DRAW);
			gpub->data(indiceCount * indiceSize, indiceBuffer, GL_STATIC_DRAW);
//			setIndicesBuffer(new AttributeBuffer(0, 1, type, false, gpub));
			m->indices(make_ptr<attribute_buffer>(0, 1, type, gpub));

			delete []indiceBuffer;
		}

//		desc->clearData();
	} catch (...) {
//		desc->clearData();
		throw exception();
	}

	return m;
}
