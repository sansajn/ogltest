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
	points,
	line_strip,
	line_loop,
	lines,
	line_strip_adjacency,
	lines_adjacency,
	triangle_strip,
	triangle_fun,
	triangles,
	triangle_strip_adjacency,
	triangles_adjacency,
	patches
};

//! \saa #glVertexAttribPointer
enum class attribute_type
{
	i8,
	ui8,
	i16,
	ui16,
	i32,
	ui32,
	f16,
	f32,
	f64,
	i32_fixed,
	i32_2_10_10_10_rev,
	ui32_2_10_10_10_rev,
	ui32_10F_11F_11F_rev,
	undefined  // TODO: je toto potrebne ?
};

//! \saa #glPolygonMode
enum class polygon_raster_mode
{
	POINT,
	LINE,
	FILL
};
