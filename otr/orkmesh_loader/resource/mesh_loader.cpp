// TODO: ork mesh loader (copy paste from ork)
#include "mesh_loader.h"
#include <memory>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert>
#include "core/ptr.h"
#include "render/meshbuffers.h"
#include "render/gpubuffer.h"
#include "render/types.h"

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
			m->mode = mesh_mode::POINTS;
		} else if (strcmp(buf, "lines") == 0) {
			m->mode = mesh_mode::LINES;
		} else if (strcmp(buf, "linesadjacency") == 0) {
			m->mode = mesh_mode::LINES_ADJACENCY;
		} else if (strcmp(buf, "linestrip") == 0) {
			m->mode = mesh_mode::LINE_STRIP;
		} else if (strcmp(buf, "linestripadjacency") == 0) {
			m->mode = mesh_mode::LINE_STRIP_ADJACENCY;
		} else if (strcmp(buf, "triangles") == 0) {
			m->mode = mesh_mode::TRIANGLES;
		} else if (strcmp(buf, "trianglesadjacency") == 0) {
			m->mode = mesh_mode::TRIANGLES_ADJACENCY;
		} else if (strcmp(buf, "trianglestrip") == 0) {
			m->mode = mesh_mode::TRIANGLE_STRIP;
		} else if (strcmp(buf, "trianglestripadjacency") == 0) {
			m->mode = mesh_mode::TRIANGLE_STRIP_ADJACENCY;
		} else if (strcmp(buf, "trianglefan") == 0) {
			m->mode = mesh_mode::TRIANGLE_FAN;
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
		attribute_type *attributeTypes = new attribute_type[attributeCount];
		bool *attributeNorms = new bool[attributeCount];

		try {
			for (unsigned int i = 0; i < attributeCount; ++i) {
				in >> attributeIds[i];
				in >> attributeComponents[i];

				in >> buf;
				if (strcmp(buf, "byte") == 0) {
					attributeTypes[i] = attribute_type::A8I;
					vertexSize += attributeComponents[i] * 1;
				} else if (strcmp(buf, "ubyte") == 0) {
					attributeTypes[i] = attribute_type::A8UI;
					vertexSize += attributeComponents[i] * 1;
				} else if (strcmp(buf, "short") == 0) {
					attributeTypes[i] = attribute_type::A16I;
					vertexSize += attributeComponents[i] * 2;
				} else if (strcmp(buf, "ushort") == 0) {
					attributeTypes[i] = attribute_type::A16UI;
					vertexSize += attributeComponents[i] * 2;
				} else if (strcmp(buf, "int") == 0) {
					attributeTypes[i] = attribute_type::A32I;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "uint") == 0) {
					attributeTypes[i] = attribute_type::A32UI;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "float") == 0) {
					attributeTypes[i] = attribute_type::A32F;
					vertexSize += attributeComponents[i] * 4;
				} else if (strcmp(buf, "double") == 0) {
					attributeTypes[i] = attribute_type::A64F;
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
						case attribute_type::A8I: {
							int ic;
							in >> ic;
							char c = (char) ic;
							memcpy(vertexBuffer + offset, &c, sizeof(char));
							offset += sizeof(char);
							break;
						}
						case attribute_type::A8UI: {
							int iuc;
							in >> iuc;
							unsigned char uc = (unsigned char) iuc;
							memcpy(vertexBuffer + offset, &uc, sizeof(unsigned char));
							offset += sizeof(unsigned char);
							break;
						}
						case attribute_type::A16I: {
							short s;
							in >> s;
							memcpy(vertexBuffer + offset, &s, sizeof(short));
							offset += sizeof(short);
							break;
						}
						case attribute_type::A16UI: {
							unsigned short us;
							in >> us;
							memcpy(vertexBuffer + offset, &us, sizeof(unsigned short));
							offset += sizeof(unsigned short);
							break;
						}
						case attribute_type::A32I: {
							int si;
							in >> si;
							memcpy(vertexBuffer + offset, &si, sizeof(int));
							offset += sizeof(int);
							break;
						}
						case attribute_type::A32UI: {
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
						case attribute_type::A32F: {
							float f;
							in >> f;
							memcpy(vertexBuffer + offset, &f, sizeof(float));
							offset += sizeof(float);
							break;
						}
						case attribute_type::A64F: {
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

		ptr<gpubuffer> gpub(new gpubuffer());
		gpub->data(vertexCount * vertexSize, vertexBuffer, buffer_usage::STATIC_DRAW);
		for (int i = 0; i < m->attribute_count(); ++i) {
			m->attribute(i)->buf(gpub);
		}
		delete[] vertexBuffer;

		unsigned int indiceCount;
		in >> indiceCount;
		unsigned int nindices = indiceCount;
		m->nindices = nindices;

		if (nindices > 0) {
			int indiceSize;
			attribute_type type;
			if (vertexCount < 256) {
				indiceSize = 1;
				type = attribute_type::A8UI;
			} else if (vertexCount < 65536) {
				indiceSize = 2;
				type = attribute_type::A16UI;
			} else {
				indiceSize = 4;
				type = attribute_type::A32UI;
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

			gpub = make_ptr<gpubuffer>();
			gpub->data(indiceCount * indiceSize, indiceBuffer, buffer_usage::STATIC_DRAW);
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
