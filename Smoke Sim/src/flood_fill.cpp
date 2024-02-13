#include "flood_fill.hpp"

void FloodFill::update(float dt) {
    t += dt;

    if (smoke_voxels.size() >= soll) {
        done = true;
    } 
    
    if (done) {
        return;
    }
    current_radius += ease_custom(dt);

    bool expanding = true;
    while (expanding) {
        size_t size_at_start = smoke_voxels.size();
        bool someone_did_something = false;
        for (uint32_t smoke_voxel_index = 0; smoke_voxel_index < size_at_start; smoke_voxel_index++) {
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    for (int z = -1; z <= 1; z++) {
                        float distanz = 0;
                        distanz = abs(x) + abs(y) + abs(z);
                        if (distanz == 0) continue;
                        if (distanz == 1) {
                            someone_did_something = set_and_add_higher_if_free_and_radius(smoke_voxels.at(smoke_voxel_index).first + glm::uvec3(x, y, z), smoke_voxels.at(smoke_voxel_index).second+voxel_size) || someone_did_something;
                        }
                        else if (distanz == 2) {
                            someone_did_something = set_and_add_higher_if_free_and_radius(smoke_voxels.at(smoke_voxel_index).first + glm::uvec3(x, y, z), smoke_voxels.at(smoke_voxel_index).second+dist_kanten) || someone_did_something;
                        }
                        else {
                            someone_did_something = set_and_add_higher_if_free_and_radius(smoke_voxels.at(smoke_voxel_index).first + glm::uvec3(x, y, z), smoke_voxels.at(smoke_voxel_index).second+dist_ecken) || someone_did_something;
                        }
                    }   
                }
            }
        }
        if (!someone_did_something && size_at_start == smoke_voxels.size()) {
            expanding = false;
        }
    }
}

float FloodFill::get_current_radius() {
    return current_radius;
}

void FloodFill::reset(glm::vec3 origin) {
//    std::cout << PREFIX_FloodFill << "Reset\n";

    std::optional<glm::uvec3> voxel_coord = voxel_world->convert_world_coordinate_to_voxel(origin);
    bool free = check_if_smoke(voxel_coord.value()) || check_if_free(voxel_coord.value());

    if (!free) return;

    for (size_t i = 0; i < smoke_voxels.size(); i++) {
        voxel_world->set_value_at_voxel_coordinate(smoke_voxels.at(i).first, VALUE_Empty);
        interesting_voxels.push({ smoke_voxels.at(i).first, VALUE_Empty });
    }
    
    smoke_bounds = { glm::uvec3(33333), glm::uvec3(0) };
    smoke_voxels.clear();
    t = 0;
    current_radius = 0;
    done = false;

    start(origin);
}

bool FloodFill::start(glm::vec3 origin) {
    origin_point = origin;
    std::optional<glm::uvec3> voxel_coord = voxel_world->convert_world_coordinate_to_voxel(origin);
    if (!voxel_coord) {
        //std::cout << PREFIX_FloodFill << "The Start Point was not valid.\n";
        done = true;
        return false;
    }
    bool free = check_if_free(voxel_coord.value());
    if (!free) {
        /*std::cout << PREFIX_FloodFill << "The Start Point " <<
            voxel_coord.value().x << " " <<
            voxel_coord.value().y << " " <<
            voxel_coord.value().z << " was not free. Contains: " << voxel_world->get_value_at_voxel_coordinate(voxel_coord.value()) << "\n";*/
        done = true;
        return false;
    }
    else {
        std::cout << PREFIX_FloodFill << "The Start Point " << 
            voxel_coord.value().x << " " <<
            voxel_coord.value().y << " " <<
            voxel_coord.value().z << " was free.\n";
    }
    set_and_add(voxel_coord.value(), VALUE_Smoke);
}

float FloodFill::ease_linear(float time) {
    return max_radius * t;
}

float FloodFill::ease_custom(float time) {
    float discrepency = std::clamp(float(soll - smoke_voxels.size()), 0.01f, (float)soll) / (float)soll;
    return discrepency * time * 10;
}

float FloodFill::ease_quad(float time) {
    if (t < 0.5) {
        return pow(2, 20 * t - 10) / 2 * max_radius;
    }
    else {
        return (2- pow(2, -20 * std::min(t, 1.f) + 10)) / 2 * max_radius;
    }
}

bool FloodFill::check_if_free(glm::uvec3 voxel_coordinate) {
    return voxel_world->get_value_at_voxel_coordinate(voxel_coordinate) == VALUE_Empty;
}

bool FloodFill::check_if_smoke(glm::uvec3 voxel_coordinate) {
    voxel_value_type val = voxel_world->get_value_at_voxel_coordinate(voxel_coordinate);
    return val > VALUE_Empty && val < VALUE_Wall;
}

bool FloodFill::check_if_free_or_higher(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    voxel_value_type vorhandener_wert = voxel_world->get_value_at_voxel_coordinate(voxel_coordinate);
    return vorhandener_wert != VALUE_Wall && vorhandener_wert > value;
}

bool FloodFill::set_and_add(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    bool valid = voxel_world->set_value_at_voxel_coordinate(voxel_coordinate, value);
    if (valid) {
        smoke_bounds.second.x = std::max(smoke_bounds.second.x, voxel_coordinate.x);
        smoke_bounds.first.x = std::min(smoke_bounds.first.x, voxel_coordinate.x);

        smoke_bounds.second.y = std::max(smoke_bounds.second.y, voxel_coordinate.y);
        smoke_bounds.first.y = std::min(smoke_bounds.first.y, voxel_coordinate.y);

        smoke_bounds.second.z = std::max(smoke_bounds.second.z, voxel_coordinate.z);
        smoke_bounds.first.z = std::min(smoke_bounds.first.z, voxel_coordinate.z);

        smoke_voxels.push_back({ voxel_coordinate, value });
        interesting_voxels.push({ voxel_coordinate, value });
        return true;
    }
            
    return false;
}

bool FloodFill::set_and_add_if_free(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    if (check_if_free(voxel_coordinate)) {
        return set_and_add(voxel_coordinate, value);
    }
    return false;
}

bool FloodFill::set_and_add_if_free_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    if (check_if_free(voxel_coordinate) && check_if_within_radius(voxel_coordinate)) {
        return set_and_add(voxel_coordinate, value);
    }
    return false;
}

bool FloodFill::set_and_add_higher_if_free_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    if (check_if_free(voxel_coordinate) && value < current_radius) {
        return set_and_add(voxel_coordinate, value);
    }
    return false;
}

bool FloodFill::set_and_add_higher_if_smoke_and_radius(glm::uvec3 voxel_coordinate, voxel_value_type value) {
    if (check_if_smoke(voxel_coordinate) && value < current_radius) {
        return set_and_add(voxel_coordinate, value);
    }
    return false;
}

bool FloodFill::check_if_within_radius(glm::uvec3 voxel_coordinate) {
    
    return glm::distance(origin_point, voxel_world->convert_voxel_to_world_coordinate(voxel_coordinate).value()) <= current_radius;
}
