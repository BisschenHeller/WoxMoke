#include "scene.hpp"
#include "transform.h"
#include "paths.hpp"


bool Scene::load_from_file(const std::string filepath) {
    std::cout << "[Scene]    Importing scene " << filepath << "\n";
	std::ifstream inputFile(PATH_res + filepath);

    // Check if the file is open
    if (!inputFile.is_open()) {
        std::cerr << "[Scene]    Error opening scene file \"" << filepath << "\" check paths.hpp for correct resource path";
        return false; // Return an error code
    }

    std::string line;
    while (std::getline(inputFile, line)) {

        if (line.starts_with("#")) continue;
        //std::cout << "[Scene] Read line " << line << "\n";

        std::istringstream iss(line);
        std::vector<std::string> tokens;

        // Read each whitespace-separated value
        do {
            std::string token;
            iss >> token;
            if (!token.empty()) {
                tokens.push_back(token);
            }
        } while (iss);

        if (tokens.at(0) == "LIGHT_SOURCE") {
            Light newlight{ glm::vec3(stof(tokens.at(1))+30, stof(tokens.at(2)),stof(tokens.at(3))+30),   // Position
                glm::vec3(stof(tokens.at(4)), stof(tokens.at(5)),stof(tokens.at(6))),   // Color
                stof(tokens.at(7))  // Power
                    };
            _lights.push_back(newlight);
        }
        else {
            try {
                tga::Obj mesh = tga::loadObj(PATH_to_Mesh(tokens.at(0)));
                _meshes.push_back(mesh);

                

                _names.push_back(tokens.at(0));
                if (tokens.at(0) == "Grenade") grenade_index = _names.size() - 1;
            }
            catch (std::exception e) {
                std::cerr << "[Scene]    Could not load mesh \"" << tokens.at(0) + ".obj\", check paths.hpp for correct resource path.\n";
                return false;
            }
            glm::vec3 position = glm::vec3(std::stof(tokens.at(1)) + 30, std::stof(tokens.at(2)), std::stof(tokens.at(3)) + 30);
            glm::vec3 rotation = glm::vec3(std::stof(tokens.at(4)), std::stof(tokens.at(5)), std::stof(tokens.at(6)));
            glm::vec3 scale = glm::vec3(std::stof(tokens.at(7)), std::stof(tokens.at(8)), std::stof(tokens.at(9)));

            _toWorlds.push_back(
                Transform::getTranslationMatrix(position)
                * Transform::getScaleMatrix(scale)
                * Transform::getRotationMatrix(rotation));

            BoundingBox bb{};
            for (size_t i = 0; i < _meshes.at(_meshes.size()-1).vertexBuffer.size(); i++) {
                glm::vec3 undisplaced = _meshes.at(_meshes.size() - 1).vertexBuffer.at(i).position;
                glm::vec3 displaced = _toWorlds.at(_toWorlds.size() - 1) * glm::vec4(undisplaced,1);
                bb.max.x = fmax(displaced.x, bb.max.x);
                bb.max.y = fmax(displaced.y, bb.max.y);
                bb.max.z = fmax(displaced.z, bb.max.z);
                
                bb.min.x = fmin(displaced.x, bb.min.x);
                bb.min.y = fmin(displaced.y, bb.min.y);
                bb.min.z = fmin(displaced.z, bb.min.z);
            }
            _boundingBboxes.push_back(bb);

            try {
                Material mat{ tokens.at(10), std::stof(tokens.at(11)) };
                _materials.push_back(mat);
            }
            catch (std::out_of_range ex) {
                std::cout << tokens.at(0) << "[Scene]    " << tokens.at(0) << " had no material assigned, assigning empty material\n";
                _materials.push_back(Material());
            }
        }
    }



    // Close the file
    inputFile.close();

    if (grenade_index == 69420) {
        std::cout << "[Scene]    Grenade index was still 69420\n";
        return false;
    }

    std::cout << "[Scene]    Meshes: " << _toWorlds.size() << ", Light Sources: " << _lights.size() << "\n";
    std::cout << "[Scene]    => Done.\n";

    return true; // Return success
}

void Scene::start_grenade(glm::vec3 start, glm::vec3 end) {
    grenade_pos = start;
    grenade_start = start;
    grenade_dest = end;
    flying = true;
}

void Scene::update_grenade(float dt) {
    if (!flying) return;
    glm::vec3 dir = glm::normalize(grenade_dest - grenade_start);
    grenade_pos += dt * dir * grenade_speed;
    _toWorlds.at(grenade_index)[3] = glm::vec4(grenade_pos, 1);
    _toWorlds.at(grenade_index) = glm::rotate(_toWorlds.at(grenade_index), 15.f*dt, glm::vec3(0.8, 0.2, 0.3));
}

bool chance_to_intersect(vec3 ray_origin, vec3 ray_direction, BoundingBox bb) {
    float tMin = (bb.min.x - ray_origin.x) / ray_direction.x;
    float tMax = (bb.max.x - ray_origin.x) / ray_direction.x;

    if (tMin > tMax) {
        float temp = tMax;
        tMax = tMin;
        tMin = temp;
    }

    float tyMin = (bb.min.y - ray_origin.y) / ray_direction.y;
    float tyMax = (bb.max.y - ray_origin.y) / ray_direction.y;

    if (tyMin > tyMax) {
        float temp = tyMax;
        tyMax = tyMin;
        tyMin = temp;
    }

    if ((tMin > tyMax) || (tyMin > tMax)) {
        return false;
    }

    if (tyMin > tMin) {
        tMin = tyMin;
    }

    if (tyMax < tMax) {
        tMax = tyMax;
    }

    float tzMin = (bb.min.z - ray_origin.z) / ray_direction.z;
    float tzMax = (bb.max.z - ray_origin.z) / ray_direction.z;

    if (tzMin > tzMax) {
        float temp = tzMax;
        tzMax = tzMin;
        tzMin = temp;
    }

    if ((tMin > tzMax) || (tzMin > tMax)) {
        return false;
    }

    return true;
}

RaycastHit moellerTrumboreIntersect(Ray ray, vec3 vertex0, vec3 vertex1, vec3 vertex2) {
    const float EPSILON = 1e-6f;

    vec3 edge1 = vertex1 - vertex0;
    vec3 edge2 = vertex2 - vertex0;
    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) {
        return false; // The ray is parallel to the triangle
	}

    float f = 1.0f / a;
    vec3 s = ray.origin - vertex0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) {
        return false;
	}

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    float t = f * dot(edge2, q);

    if (t > EPSILON)
        return {true, ray.origin + t*ray.direction, t};

    return false;
}

const RaycastHit Scene::raycast(glm::vec3 cam_pos, glm::vec3 direction) {
    RaycastHit ret{};

    for (size_t mesh_index = 0; mesh_index < _meshes.size(); mesh_index++) {

        if (!chance_to_intersect(cam_pos, direction, _boundingBboxes.at(mesh_index))) continue;

        std::vector<uint32_t> index_buffer = _meshes.at(mesh_index).indexBuffer;
        std::vector<tga::Vertex> vertex_buffer = _meshes.at(mesh_index).vertexBuffer;

        for (size_t face_index = 0; face_index < index_buffer.size(); face_index += 3) {
            std::vector<glm::vec3> vertices{};

            for (int i = 0; i < 3; i++) {
                glm::vec3 undisplaced = vertex_buffer.at(index_buffer.at(face_index+i)).position;
                glm::vec3 displacedPosition = glm::vec3(_toWorlds.at(mesh_index) * glm::vec4(undisplaced, 1));
                vertices.push_back(displacedPosition);
            }

            Ray ray{ cam_pos, direction };

            RaycastHit hm = moellerTrumboreIntersect(ray, vertices.at(0), vertices.at(1), vertices.at(2));
            
            if (hm.distance < ret.distance) {
                ret.hit = true;
                ret.world_pos = hm.world_pos;
                ret.distance = hm.distance;
            }
        }
    }
    return ret;
}

void Scene::update_bulletholes(float dt) {

    for (size_t i = 0; i < _bulletholes.size(); i++)
    {
        _bulletholes.at(i).timer += dt/2;
    }

    auto newEnd = std::remove_if(_bulletholes.begin(), _bulletholes.end(), [](Bullethole b) {return b.timer >= 1; });
    _bulletholes.erase(newEnd, _bulletholes.end());
}