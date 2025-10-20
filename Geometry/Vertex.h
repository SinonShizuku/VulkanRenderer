#pragma once
#ifdef WIN32
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#else
#include<glm1_0/glm.hpp>
#include<glm1_0/gtc/matrix_transform.hpp>
#endif

// structs
struct vertex2D {
    glm::vec2 position;
    glm::vec4 color;
};

struct texture_vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

struct push_constant_data_3 {
    glm::vec2 offset0;
    glm::vec2 offset1;
    glm::vec2 offset2;
};

struct vertex3D {
    glm::vec3 position;
    glm::vec4 color;
};

struct vertex3D_1 {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 albedo_specular;
};

// func
inline glm::mat4 flip_vertical(const glm::mat4& projection) {
    glm::mat4 _projection = projection;
    _projection[1][1] *= -1;
    return _projection;
}