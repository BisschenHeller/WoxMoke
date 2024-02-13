#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <tga/tga_utils.hpp>
#include "paths.hpp"

/*      scene.hpp
* 
*       This file and the corresponding .cpp file load the Scene.txt and put them into tga::Obj meshes, tga::Texture textures and 
*		glm::mat4 to-world matrices. It also keeps track of the light sources. 
*		The scene file furthermore handles the raycast used for smoke placement and the placement and animation of the smoke grenade
*		Object. The scene also keeps track of the currently active bulletholes and updates their timer.
* 
*       Written by Tobias Heller
*/

struct BoundingBox {
    BoundingBox() : min(glm::vec3{ INFINITY }), max(glm::vec3{ -INFINITY }) {}
    glm::vec3 min;
    glm::vec3 max;
};

struct RaycastHit {
	RaycastHit() : hit(false), world_pos(glm::vec3{ 0 }), distance(INFINITY) {}
	RaycastHit(bool nicht) : hit(nicht), world_pos(glm::vec3{ 0 }), distance(INFINITY) {}
	RaycastHit(bool nicht, glm::vec3 wo, float d) : hit(nicht), world_pos(wo), distance(d) {}
	bool hit;
	glm::vec3 world_pos;
	float distance;
};

struct Ray {
	Ray() : origin(glm::vec3{ 0 }), direction(glm::vec3{ 0 }) {}

	Ray(glm::vec3 pos, glm::vec3 dir) : origin(pos), direction(dir) {}

	glm::vec3 origin;
	glm::vec3 direction;
};

struct Material {
	Material(std::string name, float rough) : texture_name_diffuse(name), roughness(rough) {}
	Material() : texture_name_diffuse("NO_TEXTURE"), roughness(0.5f) {}
	std::string texture_name_diffuse;
	float roughness;
};

struct Light {
	Light() : position(0), color({ 1,0,1 }) {}
	Light(glm::vec3 pos, glm::vec3 col, float pow) : position(pos), color(col * pow) {}
	glm::vec3 position;
	glm::vec3 color;
};

struct Bullethole {
	Bullethole(glm::vec3 orig, glm::vec3 dir) : origin(orig), direction(dir), timer(0.0f) {}
	glm::vec3 origin;
	glm::vec3 direction;
	float timer;
};

class Scene {

public:
	Scene() : 
		_toWorlds({}), _meshes({}), _names({}), _materials({}), _lights({}), _bulletholes({}), grenade_dest({ 0 }), grenade_pos({ 0 }), grenade_start({ 0 }), flying(false) {
		
	}

	float grenade_speed = 20.0f;

	std::vector<Bullethole> _bulletholes;
	std::vector<Light> _lights;
	std::vector<Material> _materials;
	std::vector<glm::mat4> _toWorlds;
	std::vector<tga::Obj> _meshes;
	std::vector<std::string> _names;
	std::vector<BoundingBox> _boundingBboxes;

	uint32_t grenade_index = 69420;

	void start_grenade(glm::vec3 start, glm::vec3 end);

	void update_grenade(float dt);

	bool grenade_arrived() {
		if (!flying) return false;
		if (glm::distance(grenade_pos, grenade_dest) < 0.5) {
			flying = false;
			return true;
		}
		return false;
	}

	const glm::vec3 get_grenade_pos() { return grenade_pos; };

	bool load_from_file(const std::string filepath);

	const RaycastHit raycast(glm::vec3 cam_pos, glm::vec3 direction);

	void update_bulletholes(float dt);
private:
	bool flying;
	glm::vec3 grenade_pos;
	glm::vec3 grenade_start;
	glm::vec3 grenade_dest;
};