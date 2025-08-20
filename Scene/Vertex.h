#pragma once
#ifdef WIN32
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#else
#include<glm1_0/glm.hpp>
#include<glm1_0/gtc/matrix_transform.hpp>
#endif


struct vertex {
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