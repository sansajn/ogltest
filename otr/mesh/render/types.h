#pragma once

//! Rozhoduje o tom, kam sa ulozia mesh data.
enum class mesh_usage
{
	CPU,
	GPU_STATIC,
	GPU_DYNAMIC,
	GPU_STREAM
};

//! \saa #glDrawElements
enum class mesh_mode
{
	POINTS,
	LINE_STRIP,
	LINE_LOOP,
	LINES,
	LINE_STRIP_ADJACENCY,
	LINES_ADJACENCY,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	TRIANGLES,
	TRIANGLE_STRIP_ADJACENCY,
	TRIANGLES_ADJACENCY,
	PATCHES
};

//! \saa #glVertexAttribPointer
enum class attribute_type
{
	A8I,
	A8UI,
	A16I,
	A16UI,
	A32I,
	A32UI,
	A16F,
	A32F,
	A64F,
	A32I_FIXED,
	A32_2_10_10_10_REV,
	A32UI_2_10_10_10_REV,
	A32UI_10F_11F_11F_REV,
	UNDEFINED
};

//! \saa #glPolygonMode
enum class polygon_raster_mode
{
	POINT,
	LINE,
	FILL
};
