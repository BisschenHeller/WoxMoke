#pragma once
#include <queue>
#include "voxel.hpp"
#include <algorithm>

#define PREFIX_FloodFill ("[FloodFill]  ")

/*      FloodFill.hpp
* 
*       This File and the corresponding .cpp file handle the physics side of the smoke (not including
*       bullet holes, which are done in the pathtracing.comp shader and scene.cpp and purely cosmetic).
*
*       The main algorithm used to fill a space with smoke is akin to floodfill, but stores the approx. 
*       distance to the smoke origin in every smoke voxel, enabling the smoke to dynamically flow where 
*       it finds open space.
* 
*       Only voxels that change between 2 update() calls are loaded into the "interesting_voxels" queue
*       and sent to the path tracing shader.
* 
*       Written by Tobias Heller
*/

using voxel_value_type = float;

class FloodFill {
public:
    FloodFill(float radius, float voxel_s, VoxelWorld<voxel_value_type>* vw) 
        : voxel_size(voxel_s), dist_kanten(sqrtf(powf(voxel_s,2)*2)), dist_ecken(sqrtf(powf(voxel_s,2)*3)), soll(1800),
        origin_point({0}), done(true), max_radius(radius), current_radius(0), smoke_voxels({}), voxel_world(vw), t(0),
        smoke_bounds({glm::uvec3(33333), glm::uvec3(0)})
    {}

    
public:
    size_t get_smoke_count() { return smoke_voxels.size(); }

    void reset(glm::vec3 origin);

    void update(float dt);

    bool start(glm::vec3 origin);

    const bool is_done() { return done; }

    std::queue<std::pair<glm::uvec3, voxel_value_type>> interesting_voxels;

    float get_current_radius();

    const std::pair<glm::uvec3, glm::uvec3> get_smoke_bounds() { return smoke_bounds; };

private:
    uint32_t soll;
    bool done;
    float t;
    float max_radius;
    float current_radius;
    float voxel_size;
    float dist_kanten;
    float dist_ecken;
    glm::vec3 origin_point;

    std::vector<std::pair<glm::uvec3, voxel_value_type>> smoke_voxels;
    VoxelWorld<voxel_value_type>* voxel_world;

    float ease_linear(float t);
    float ease_quad(float t);
    float ease_custom(float t);

    
	std::pair<glm::uvec3, glm::uvec3> smoke_bounds;
    
    bool check_if_free(glm::uvec3 voxel_coordinate);
    bool check_if_smoke(glm::uvec3 voxel_coordinate);
    bool check_if_free_or_higher(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool set_and_add(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool set_and_add_if_free(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool set_and_add_if_free_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool set_and_add_higher_if_free_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool set_and_add_higher_if_smoke_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value);
    bool check_if_within_radius(glm::uvec3 voxel_coordinate);
};