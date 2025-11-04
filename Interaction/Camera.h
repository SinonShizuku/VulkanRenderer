#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>

class Camera {
private:
    float fov;
    float z_near,z_far;

    void update_view_matrix() {
        glm::mat4 current_matrix = matrices.view;
        glm::mat4 rotM = glm::mat4(1.0f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(rotation.x* (flip_y ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = position;
        if (flip_y) translation.y = -translation.y;
        transM = glm::translate(glm::mat4(1.0f), translation);

        matrices.view = transM * rotM;

        view_pos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        updated = current_matrix != matrices.view;
    }

public:
    glm::vec3 rotation = glm::vec3();
    glm::vec3 position = glm::vec3();
    glm::vec4 view_pos = glm::vec4();

    float rotation_speed = 1.0f;
    float movement_speed = 1.0f;

    bool updated = true;
    bool flip_y = false;

    struct{
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;

    // get func
    [[nodiscard]] float get_near_clip() const {
        return z_near;
    }
    [[nodiscard]] float get_far_clip() const {
        return z_far;
    }

    // set func
    void set_perspective(float fov, float aspect, float znear, float zfar) {
        glm::mat4 current_matrix = matrices.perspective;
        this->fov = fov;
        this->z_near = znear;
        this->z_far = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
        if (flip_y) matrices.perspective[1][1] *= -1.f;
        updated = current_matrix != matrices.perspective;
    }

    void update_aspect_ratio(float aspect) {
        glm::mat4 current_matrix = matrices.perspective;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, z_near, z_far);
        if (flip_y) matrices.perspective[1][1] *= -1.f;
        updated = current_matrix != matrices.perspective;
    }

    void set_position(glm::vec3 pos) {
        this->position = pos;
        update_view_matrix();
    }

    void set_rotation(glm::vec3 rot) {
        this->rotation = rot;
        update_view_matrix();
    }

    void rotate(glm::vec3 delta) {
        this->rotation += delta;
        update_view_matrix();
    }

    void set_translation(glm::vec3 translation) {
        this->position = translation;
        update_view_matrix();
    }

    void translate(glm::vec3 delta) {
        this->position += delta;
        update_view_matrix();
    }

    void set_rotation_speed(float rotation_speed) {
        this->rotation_speed = rotation_speed;
    }

    void set_movement_speed(float movement_speed) {
        this->movement_speed = movement_speed;
    }

};