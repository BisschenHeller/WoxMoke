#include "voxel.hpp"

// If a mesh only barely scapes a voxel, better not to count it as overlapping
#define EPSILON (0.01f)

template class VoxelWorld<float>;

template <typename T>
void VoxelWorld<T>::save_voxels_as_mesh() {
	std::cout << PREFIX << "Converting Array To Mesh...\n";
	voxel_mesh = tga::Obj();
	uint32_t running_vertex_count = 0;
	uint32_t running_face_count = 0;
	for (uint32_t voxel_index = 0; voxel_index < voxel_count; voxel_index++) {
		if (voxels[voxel_index] != 0) {
			glm::uvec3 voxel_koords = convert_index_to_voxel_coordinates(voxel_index).value();
			glm::vec3 world_koord = convert_voxel_to_world_coordinate(voxel_koords).value();

			for (int i = 0; i < 8; i++) {
				tga::Vertex v{};
				
				v.position = world_koord;

				v.position.x += (i % 8 < 4 ? voxel_size : -voxel_size) / 2;
				v.position.y += (i % 4 < 2 ? voxel_size : -voxel_size) / 2;
				v.position.z += (i % 2 < 1 ? voxel_size : -voxel_size) / 2;

				v.position.x -= 30;
				v.position.z -= 30;

				voxel_mesh.vertexBuffer.push_back(v);
			}
			
			voxel_mesh.indexBuffer.push_back(1 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(0 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(2 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(3 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(1 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(2 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(5 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(1 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(3 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(7 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(5 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(3 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(4 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(5 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(7 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(6 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(4 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(7 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(0 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(4 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(6 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(2 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(0 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(6 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(4 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(0 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(1 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(5 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(4 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(1 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(2 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(6 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(3 + running_vertex_count);

			voxel_mesh.indexBuffer.push_back(7 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(3 + running_vertex_count);
			voxel_mesh.indexBuffer.push_back(6 + running_vertex_count);
			running_vertex_count += 8; // TODO da sich die Zahlen nicht ändern könnte man einfach im sheder ein modulo für die Face indecies verwenden statt 100 mal dieselben face indices zu pushen
		}
	}
	
	
		std::cout << PREFIX << "    Writing mesh to .obj file\n";
		std::ofstream voxel_obj;
		voxel_obj.open("../../../../res/Meshes/Voxels.obj");
		if (voxel_obj.is_open()) {
			for (size_t i = 0; i < voxel_mesh.vertexBuffer.size(); i++) {
				glm::vec3 pos = voxel_mesh.vertexBuffer.at(i).position;
				voxel_obj << "v " << std::to_string(pos.x) << " " << std::to_string(pos.y) << " " << std::to_string(pos.z) << "\n";
			}
			for (size_t i = 0; i < voxel_mesh.indexBuffer.size(); i += 3) {
				voxel_obj << "f " << (voxel_mesh.indexBuffer.at(i) + 1) << " " << (voxel_mesh.indexBuffer.at(i + 1) + 1) << " " << (voxel_mesh.indexBuffer.at(i + 2) + 1) << "\n";
			}
			voxel_obj.close();
		}
		else {
			std::cout << "Could not open Voxels.obj\n";
		}
	
	std::cout << PREFIX << "    => Done.\n";
}

template <typename T>
void VoxelWorld<T>::save_voxels_as_vxl() {
	std::cout << PREFIX << "Saving voxel data as vxl file...\n";

	std::ofstream voxel_vxl;
	voxel_vxl.open("../../../../res/Voxels.vxl");
	if (voxel_vxl.is_open()) {
		for (uint32_t i = 0; i < voxel_count; i++) {
			voxel_vxl << std::to_string(get_value_at_index(i)) << " ";
		}
		voxel_vxl.close();
		std::cout << PREFIX << "    => Done.\n";
	}
	else {
		std::cout << "Could not open Voxels.vxl\n";
	}
}

template <typename T>
bool VoxelWorld<T>::load_from_vxl() {
	std::cout << PREFIX << "Loading voxel data from vxl file...\n";
	try {
		std::ifstream voxel_vxl;
		voxel_vxl.open("../../../../res/Voxels.vxl");
		if (!voxel_vxl.is_open()) {
			std::cout << PREFIX << "    Could not open /res/Voxels.vxl\n";
			return false;
		}

		for (uint32_t i = 0; i < voxel_count; i++) {
			std::string number;
			voxel_vxl >> number;
			set_value_at_index(i, stof(number));
		}
		voxel_vxl.close();
		std::cout << PREFIX << "=> Done.\n";
		return true;
	}
	catch (std::exception e) {
		std::cout << PREFIX << "    Error reading /res/Voxels.vxl: " << e.what() << "\n";
		return false;
	}
}

template <typename T>
void VoxelWorld<T>::calculate_voxel_world(std::vector<tga::Obj> meshes, std::vector<glm::mat4> toWorlds, std::vector<std::string> names) {
	
	std::cout << PREFIX << "Performing Trieangle SAT for " << voxel_count << " voxels...\n";

	for (uint32_t i = 0; i < voxel_count; i++) {
		voxels[i] = 0;
	}

	struct Axis {
		Axis(float _min, float _max) : min(_min), max(_max) {

		}
		float min = INFINITY;
		float max = -INFINITY;
	};

	for (size_t mesh_index = 0; mesh_index < meshes.size(); mesh_index++) {
		if (names.at(mesh_index).starts_with("IGNORE_")) {
			std::cout << PREFIX << "Skipping " << names.at(mesh_index).substr(7) << "\n";
			continue;
		}
		//
		// ______________________________ Voxel Bounds der Mesh berechnen ______________________________
		//
		Axis mesh_bounds_x{ INFINITY, -INFINITY };
		Axis mesh_bounds_y{ INFINITY, -INFINITY };
		Axis mesh_bounds_z{ INFINITY, -INFINITY };

		for (int vertex_index = 0; vertex_index < meshes.at(mesh_index).vertexBuffer.size(); vertex_index++) {
			glm::vec3 undisplaced = meshes.at(mesh_index).vertexBuffer.at(vertex_index).position;
			glm::vec3 displacedPosition = glm::vec3(toWorlds.at(mesh_index) * glm::vec4(undisplaced, 1));
			/*std::cout << "Displaced: " <<
				std::to_string(displacedPosition.x) << " " <<
				std::to_string(displacedPosition.y) << " " <<
				std::to_string(displacedPosition.z) << "\n";*/
			if (displacedPosition.x > mesh_bounds_x.max) mesh_bounds_x.max = displacedPosition.x;
			if (displacedPosition.x < mesh_bounds_x.min) mesh_bounds_x.min = displacedPosition.x;

			if (displacedPosition.y > mesh_bounds_y.max) mesh_bounds_y.max = displacedPosition.y;
			if (displacedPosition.y < mesh_bounds_y.min) mesh_bounds_y.min = displacedPosition.y;

			if (displacedPosition.z > mesh_bounds_z.max) mesh_bounds_z.max = displacedPosition.z;
			if (displacedPosition.z < mesh_bounds_z.min) mesh_bounds_z.min = displacedPosition.z;
		}
		glm::uvec3 min_voxel;
		glm::uvec3 max_voxel;
		try {
			glm::vec3 epsilon_min{ mesh_bounds_x.min, mesh_bounds_y.min, mesh_bounds_z.min};
			glm::vec3 epsilon_max{ mesh_bounds_x.max, mesh_bounds_y.max, mesh_bounds_z.max};
			min_voxel = convert_world_coordinate_to_voxel(epsilon_min).value();
			max_voxel = convert_world_coordinate_to_voxel(epsilon_max).value();
			std::cout << PREFIX << names.at(mesh_index) << " goes from voxel " <<
				min_voxel.x << " " <<
				min_voxel.y << " " <<
				min_voxel.z << " to voxel " <<
				max_voxel.x << " " <<
				max_voxel.y << " " <<
				max_voxel.z << ".\n";
		}
		catch (std::bad_optional_access e) {
			std::cout << PREFIX << names.at(mesh_index) << " out of voxel Bounds.\n";
			continue;
		}

		uint32_t voxels_in_OOBB =
			(max_voxel.x - min_voxel.x) *
			(max_voxel.y - min_voxel.y) *
			(max_voxel.z - min_voxel.z);
		std::cout << PREFIX << "    | (0%)=";
		float voxels_done = 0;
		std::vector<bool> landmarks{ true, true, true, true };
		for (uint32_t voxel_x = min_voxel.x; voxel_x <= max_voxel.x; voxel_x++) {
			for (uint32_t voxel_y = min_voxel.y; voxel_y <= max_voxel.y; voxel_y++) {
				for (uint32_t voxel_z = min_voxel.z; voxel_z <= max_voxel.z; voxel_z++) {
					voxels_done += 1;
					if (voxels_done / voxels_in_OOBB > 0.2 && landmarks.at(0)) { std::cout << "=(20%)="; landmarks.at(0) = false; }
					if (voxels_done / voxels_in_OOBB > 0.4 && landmarks.at(1)) { std::cout << "=(40%)="; landmarks.at(1) = false; }
					if (voxels_done / voxels_in_OOBB > 0.6 && landmarks.at(2)) { std::cout << "=(60%)="; landmarks.at(2) = false; }
					if (voxels_done / voxels_in_OOBB > 0.8 && landmarks.at(3)) { std::cout << "=(80%)="; landmarks.at(3) = false; }
					
					if (get_value_at_voxel_coordinate(glm::uvec3{ voxel_x, voxel_y, voxel_z })!= VALUE_Empty) continue;

					for (size_t face_index = 0; face_index < meshes.at(mesh_index).indexBuffer.size(); face_index += 3) {
						
						std::vector<glm::vec3> points_on_face{};

						std::vector<Axis> axes{ 
							{ INFINITY, -INFINITY }, // Global X
							{ INFINITY, -INFINITY }, // Global Y
							{ INFINITY, -INFINITY }, // Global Z
							{ -INFINITY , 0 },	     // Face normal
							{ INFINITY, 0 }, // Cross ( n, 0 - 1 )
							{ INFINITY, 0 }, // Cross ( n, 1 - 2 )
							{ INFINITY, 0 }, // Cross ( n, 2 - 0 )
						};
						for (size_t vertex_index = face_index; vertex_index < face_index + 3; vertex_index++) {
							tga::Vertex vertex = meshes.at(mesh_index).vertexBuffer.at(meshes.at(mesh_index).indexBuffer.at(vertex_index));
							glm::vec3 undisplaced = vertex.position;
							glm::vec3 displacedPosition = glm::vec3(toWorlds.at(mesh_index) * glm::vec4(undisplaced, 1));
							// Global Axes
							if (displacedPosition.x > axes.at(0).max) axes.at(0).max = displacedPosition.x;
							if (displacedPosition.x < axes.at(0).min) axes.at(0).min = displacedPosition.x;

							if (displacedPosition.y > axes.at(1).max) axes.at(1).max = displacedPosition.y;
							if (displacedPosition.y < axes.at(1).min) axes.at(1).min = displacedPosition.y;

							if (displacedPosition.z > axes.at(2).max) axes.at(2).max = displacedPosition.z;
							if (displacedPosition.z < axes.at(2).min) axes.at(2).min = displacedPosition.z;

							points_on_face.push_back(displacedPosition);
						}
						glm::vec3 face_normal = glm::normalize(glm::cross(points_on_face.at(0) - points_on_face.at(1), points_on_face.at(2) - points_on_face.at(1)));

						glm::vec3 voxel_center = convert_voxel_to_world_coordinate({voxel_x, voxel_y, voxel_z}).value();
						glm::vec3 voxel_max{ voxel_center.x + voxel_size / 2, voxel_center.y + voxel_size / 2, voxel_center.z + voxel_size / 2 };
						glm::vec3 voxel_min{ voxel_center.x - voxel_size / 2, voxel_center.y - voxel_size / 2, voxel_center.z - voxel_size / 2 };

						//
						// Along Normal
						//

						Axis voxel_from_normal{ INFINITY, -INFINITY };
						{
							glm::vec3 P_Q = voxel_center - points_on_face.at(0);

							float distance = -glm::dot(P_Q, face_normal); // Da N bereits normalisiert

							if (distance < voxel_from_normal.min) voxel_from_normal.min = distance;
							if (distance > voxel_from_normal.max) voxel_from_normal.max = distance;
						}

						//
						// Along Cross products of normal and one edge
						//

						std::vector<Axis> normal_cross_voxels{ 3, {INFINITY, -INFINITY} };
						for (uint32_t i = 0; i < 3; i++) {
							glm::vec3 direction = glm::normalize(glm::cross(face_normal, points_on_face.at(i) - points_on_face.at((i + 1) % 3)));
							// Span of triangle
							glm::vec3 P_T = points_on_face.at((i + 2) % 3) - points_on_face.at(i);
							float span = glm::dot(P_T, direction);
							axes.at(4+i).min = -span;

							// span of voxel
							glm::vec3 P_Q = voxel_center - points_on_face.at(i);
							float distance = -glm::dot(P_Q, direction);

							if (distance - EPSILON*10 < normal_cross_voxels.at(i).min) normal_cross_voxels.at(i).min = distance - EPSILON*10;
							if (distance + EPSILON*10 > normal_cross_voxels.at(i).max) normal_cross_voxels.at(i).max = distance + EPSILON*10;
						}

						std::vector<Axis> voxel_axes{ 
							{voxel_min.x, voxel_max.x}, 
							{voxel_min.y, voxel_max.y},
							{voxel_min.z, voxel_max.z}, 
							voxel_from_normal, 
							normal_cross_voxels.at(0),
							normal_cross_voxels.at(1),
							normal_cross_voxels.at(2)
						};

						bool sa_existiert = false;
						for (int axis_index = 0; axis_index < 7; axis_index++) {

							bool seperates = voxel_axes.at(axis_index).max <= axes.at(axis_index).min || axes.at(axis_index).max <= voxel_axes.at(axis_index).min;

							if (seperates) {
								sa_existiert = true;
								break;
							}
						}
						if (!sa_existiert) {
							set_value_at_voxel_coordinate({ voxel_x, voxel_y, voxel_z }, VALUE_Wall);
						}

					}
				}
			}

		}
		std::cout << "=(100%) | (" << mesh_index << "/" << meshes.size() << ") Done.\n";
	}

	std::cout << PREFIX << "=> Done.\n";
	
	if (BOOL_OUTPUT_OBJ) save_voxels_as_mesh();
	if (BOOL_OUTPUT_VXL) save_voxels_as_vxl();
}

template <typename T>
std::optional<uint32_t> VoxelWorld<T>::convert_voxel_coordinates_to_index(glm::uvec3 voxel_coordinates) {
	if (voxel_coordinates.x >= voxel_count_x) {
		//std::cout << PREFIX << "convert_voxel_coordinates_to_index() was called with too high indecies. X Can be "	<< (voxel_count_x-1) << " at most, was " << voxel_coordinates.x << ".\n";
		return std::nullopt;
	}
	if (voxel_coordinates.y >= voxel_count_y) {
		//std::cout << PREFIX << "convert_voxel_coordinates_to_index() was called with too high indecies. Y Can be "	<< (voxel_count_y-1) << " at most, was " << voxel_coordinates.y << ".\n";
		return std::nullopt;
	}
	if (voxel_coordinates.z >= voxel_count_z) {
		//std::cout << PREFIX << "convert_voxel_coordinates_to_index() was called with too high indecies. Z Can be "<< (voxel_count_z-1) << " at most, was " << voxel_coordinates.z << ".\n";
		return std::nullopt;
	}
	return std::make_optional<uint32_t>(voxel_coordinates.x * voxel_count_y * voxel_count_z + voxel_coordinates.y * voxel_count_z + voxel_coordinates.z);
}

template <typename T>
std::optional<glm::uvec3> VoxelWorld<T>::convert_index_to_voxel_coordinates(uint32_t index) {
	if (index >= voxel_count) {
		std::cout << PREFIX << "convert_index_to_voxel_coordinates() was called with too high index. Maximum index is " << (voxel_count - 1) << ", was " << index << "\n";
		return std::nullopt;
	}
	
	uint32_t x = index / (voxel_count_y * voxel_count_z);
	index -= x * voxel_count_y * voxel_count_z;
	

	uint32_t y = index / voxel_count_z;
	index -= y * voxel_count_z;

	uint32_t z = index;
	return std::make_optional<glm::uvec3>(glm::uvec3(x, y, z));
}

// Returns VALUE_Out_Of_Bounds when called outside the world's boundaries.
template <typename T>
const T VoxelWorld<T>::get_value_at_voxel_coordinate(glm::uvec3 voxel_coordinates) {
	std::optional<int> index = convert_voxel_coordinates_to_index(voxel_coordinates);
	if (index) {
		return voxels[index.value()];
	}
	else {
		return VALUE_Out_Of_Bounds;
	}
}

// Returns VALUE_Out_Of_Bounds when called outside the world's boundaries.
template <typename T>
const T VoxelWorld<T>::get_value_at_world_coordinate(glm::vec3 coordinate) {
	std::optional<glm::uvec3> voxel_coordinates = convert_world_coordinate_to_voxel(coordinate);
	if (voxel_coordinates) {
		return get_value_at_voxel_coordinate(voxel_coordinates.value());
	}
	else {
		return VALUE_Out_Of_Bounds;
	}
}

template <typename T>
bool VoxelWorld<T>::set_value_at_voxel_coordinate(glm::uvec3 voxel_coordinate, T neuer_wert) {
	std::optional<uint32_t> index = convert_voxel_coordinates_to_index(voxel_coordinate);
	if (index) {
		voxels[index.value()] = neuer_wert;
		return true;
	}
	else {
		return false;
	}
}

template <typename T>
bool VoxelWorld<T>::set_value_at_world_coordinate(glm::vec3 coordinate, T value) {
	// Damit die Ecken safe drinnen sind
	/*if (abs(coordinate.x) * 2 == world_width) {
		coordinate.x += coordinate.x < 0 ? 0.001f : -0.001f;
	}
	if (abs(coordinate.y) == world_height) {
		coordinate.y -= 0.001f;
	}
	if (abs(coordinate.z) * 2 == world_depth) {
		coordinate.z += coordinate.z < 0 ? 0.001f : -0.001f;
	}*/

	std::optional<glm::uvec3> voxel_coordinates = convert_world_coordinate_to_voxel(coordinate);
	if (voxel_coordinates) {
		return set_value_at_voxel_coordinate(voxel_coordinates.value(), value);
	} else {
		return false;
	}
}

template <typename T> 
bool VoxelWorld<T>::set_value_at_index(uint32_t index, T value) 
{
	if (index < voxel_count) {
		voxels[index] = value;
		return true;
	}
	else {
		return false;
	}
}

template <typename T>
std::optional<glm::uvec3> VoxelWorld<T>::convert_world_coordinate_to_voxel(glm::vec3 coordinate) {
	if (coordinate.x >= world_width) coordinate.x = world_width - EPSILON;
	else if (coordinate.x < 0) coordinate.x = 0+EPSILON;

	if (coordinate.y >= world_height) coordinate.y = world_height - EPSILON;
	else if (coordinate.y < 0) coordinate.y = 0+EPSILON;

	if (coordinate.z >= world_depth) coordinate.z = world_depth - EPSILON;
	else if (coordinate.z < 0) coordinate.z = 0+EPSILON;
		
	
	uint32_t x_voxel = static_cast<uint32_t>(floorf(coordinate.x / voxel_size));
	uint32_t y_voxel = static_cast<uint32_t>(floorf(coordinate.y / voxel_size));
	uint32_t z_voxel = static_cast<uint32_t>(floorf(coordinate.z / voxel_size));
	
	return std::make_optional<glm::uvec3>({ x_voxel, y_voxel, z_voxel });
}

template <typename T>
std::optional<glm::vec3> VoxelWorld<T>::convert_voxel_to_world_coordinate(glm::uvec3 voxel_coordinates) {
	float x = (voxel_coordinates.x * voxel_size) + voxel_size/2;
	float y = (voxel_coordinates.y * voxel_size) + voxel_size/2;
	float z = (voxel_coordinates.z * voxel_size) + voxel_size/2;

	return glm::vec3(x, y, z);
}

bool check_same_values(glm::vec3 a, glm::uvec3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool check_same_values(glm::uvec3 a, glm::vec3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool check_same_values(glm::vec3 a, glm::vec3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool check_same_values(glm::uvec3 a, glm::uvec3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
template <typename I, typename O>
bool VoxelWorld<T>::test_conversion(I input, O expected_output) {
	std::cout << PREFIX_TEST << "Testing " << 
		(std::is_same<O, glm::uvec3>::value? "world coordinate (" : "voxel (")
		<< std::to_string(input.x) << " "
		<< std::to_string(input.y) << " "
		<< std::to_string(input.z) << "), expecting ("
		<< std::to_string(expected_output.x) << " "
		<< std::to_string(expected_output.y) << " "
		<< std::to_string(expected_output.z) << ") as output.";

	bool correct = false;
	{
		if (std::is_same<I, glm::vec3>::value) {
			glm::uvec3 result = convert_world_coordinate_to_voxel(input).value();
			std::cout << "    \toutput v (" <<
				result.x << " " <<
				result.y << " " <<
				result.z << ")";

			correct = check_same_values(result, static_cast<glm::uvec3>(expected_output));
		}
		else {
			glm::vec3 result = convert_voxel_to_world_coordinate(input).value();
			std::cout << "    \toutput wc (" <<
				std::to_string(result.x) << " " <<
				std::to_string(result.y) << " " <<
				std::to_string(result.z) << ")";
			correct = check_same_values(result, static_cast<glm::vec3>(expected_output));
		}
	}
	
	if (correct) {
		std::cout << "    \tSUCCESS\n";
		return true;
	}
	else {
		std::cout << "    \tFAILED\n";
		return false;
	}
}

template <typename T>
void VoxelWorld<T>::test_conversions() {
	
    VoxelWorld five_five_five(5, 5, 5, 0.5);
	
	// World to voxel
	five_five_five.test_conversion(glm::vec3(0, 0, 0), glm::uvec3(0,0,0));
	five_five_five.test_conversion(glm::vec3(0.4, 0.4, 0.4), glm::uvec3(0,0,0));
	five_five_five.test_conversion(glm::vec3(0.4, 0.6, 0.4), glm::uvec3(0,1,0));
	five_five_five.test_conversion(glm::vec3(0.5001, 0.6, 0.511), glm::uvec3(1,1,1));
}

template <typename T>
std::string VoxelWorld<T>::to_string() {
	std::string ret = "";
	ret += "Voxel Count: " + std::to_string(voxel_count)
		+ " (x" + std::to_string(voxel_count_x)
		+ " y" + std::to_string(voxel_count_y)
		+ " z" + std::to_string(voxel_count_z) + ")\n";
	for (uint32_t y = 0; y < voxel_count_y; y++) {
		ret += "y=" + std::to_string(y) + "\n";
		for (uint32_t z = 0; z < voxel_count_z; z++) {
			for (uint32_t x = 0; x < voxel_count_x; x++) {
				ret += std::to_string(get_value_at_voxel_coordinate(glm::uvec3{ x, y, z }));
				ret += " ";
				if (x == voxel_count_x - 1) {
					ret += "\n";
				}
			}
		}
		ret += "\n";
	}
	return ret;
}