#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <tga/tga_utils.hpp>
#include "scene.hpp"

#define PREFIX ("[VoxelWorld]    ")
#define PREFIX_TEST ("[VoxelWorld Testing]    ")
#define BOOL_READ_FROM_VXL (true)
#define BOOL_OUTPUT_OBJ (true)
#define BOOL_OUTPUT_VXL (true)

#define VALUE_Wall (500.0f)
#define VALUE_Out_Of_Bounds (69420.f)
#define VALUE_Smoke (1.0f)
#define VALUE_Empty (0.0f)

/*      voxel.hpp
* 
*       This file and the corresponding .cpp file handle the underlying logic of the voxel world including 
*		the calculation of the voxel world through a series of Seperationg Axis Tests.
* 
*		In the SAT, every triangle of every mesh is tested against all possible voxels contained in the 
*		AABB of the mesh. Seven axes are considered: the three global coordinate axes, the outward-facing 
*		normal of the triangle, and the three cross products of the three edges with the triangle normal.
*		As of 4.2.2024, the sat for the last 4 axes considers only the center of a voxel, not the complete
*		boundary of the voxel's 8 vertices.
* 
*		Only tested for voxel size 0.5
* 
*       Written by Tobias Heller
*/

struct VoxelInfo {
        glm::uvec3 voxel_count;
		glm::vec3 smoke_min_coord;
		glm::vec3 smoke_max_coord;
        glm::vec3 world_measurements;
        glm::vec3 voxel_size;
		glm::vec3 smoke_center;
		glm::vec3 smoke_radius;
		glm::vec3 swipe;
};

template <typename value_type>
class VoxelWorld {
public:
	VoxelWorld(float scene_width, float scene_height, float scene_depth, float voxel_s) 
		: world_width(scene_width), world_height(scene_height), world_depth(scene_depth), voxel_size(voxel_s)
	{
		voxel_count_x = static_cast<uint32_t>(world_width / voxel_size);
		voxel_count_y = static_cast<uint32_t>(world_height / voxel_size);
		voxel_count_z = static_cast<uint32_t>(world_depth / voxel_size);
		voxel_count = voxel_count_x * voxel_count_y * voxel_count_z;

		voxels = std::make_unique<value_type[]>(voxel_count);
	}

	void calculate_voxel_world(std::vector<tga::Obj> meshes, std::vector<glm::mat4> toWorlds, std::vector<std::string> names);

	//
	//_________________________________________ Setters ______________________________________________
	//

	bool set_value_at_voxel_coordinate(glm::uvec3 voxel_coordinate, value_type value);
	bool set_value_at_world_coordinate(glm::vec3 coordinate, value_type value);
	bool set_value_at_index(uint32_t index, value_type value);

	//
	//_________________________________________ Getters ______________________________________________
	//

	// Calling a getter outside the world's bounds will return VALUE_Out_Of_Bounds_Void
	const value_type get_value_at_voxel_coordinate(glm::uvec3 voxel_coordinate);
	const value_type get_value_at_world_coordinate(glm::vec3 coordinate);
	const value_type get_value_at_index(uint32_t index) { return index >= voxel_count ? VALUE_Out_Of_Bounds : voxels[index]; };

	glm::uvec3 get_voxel_counts() { return glm::uvec3(voxel_count_x, voxel_count_y, voxel_count_z); }
	uint32_t get_voxel_count() { return voxel_count; }
	float get_voxel_size() { return voxel_size; }
	glm::vec3 get_world_measurements() { return glm::vec3(world_width, world_height, world_depth); }
	

	//
	//_________________________________________ Conversions __________________________________________
	//

	std::optional<glm::uvec3> convert_world_coordinate_to_voxel(glm::vec3 coordinate);
	std::optional<glm::vec3>  convert_voxel_to_world_coordinate(glm::uvec3 voxel);

	std::optional<uint32_t>   convert_voxel_coordinates_to_index(glm::uvec3 voxel_coordinate);
	std::optional<glm::uvec3> convert_index_to_voxel_coordinates(uint32_t index);

	std::string to_string();

	bool load_from_vxl();
private:
	static void test_conversions();
	

	std::unique_ptr<value_type[]> voxels;

	float world_width = 60;
	float world_depth = 60;
	float world_height = 10;
	float voxel_size = 0.5f;

	tga::Obj voxel_mesh;

	uint32_t voxel_count_x;
	uint32_t voxel_count_y;
	uint32_t voxel_count_z;
	uint32_t voxel_count;

	void save_voxels_as_mesh();
	void save_voxels_as_vxl();
	
	
	template <typename I, typename O>
	bool test_conversion(I input, O expected_output);
};