//
// Created by Ninter6 on 2024/7/15.
//

#include "camera.hpp"

Camera::Camera(mathpls::vec3 pos, mathpls::vec3 target)
: position(pos), forward((target - pos).normalized()) {}

void Camera::init_event(Eventor& eventor) {
    Event evt{};

    evt.MouseMove = true;
    eventor.add_event(evt, [&](double x, double y) {
        auto dt = mathpls::radians(x);
        auto& [x0, y0, z0] = forward.asArray;
        x0 = cos(dt)*x0 - sin(dt)*z0;
        z0 = sin(dt)*x0 + cos(dt)*z0;
        y0 -= y * .02;
        forward.normalize();
        is_dirty = true;
    });

    evt.MouseMove = std::nullopt;
    evt.NormalKey = 'F';
    eventor.add_event(evt, [&]() {
        forward = {0, 0, 1};
        is_dirty = true;
    });

    evt.NormalKey = 'W';
    eventor.add_event(evt, [&]() {
        position += forward * .1f;
        is_dirty = true;
//        std::cout << camera->position.x << " " << camera->position.z << "\n";
    });
    evt.NormalKey = 'S';
    eventor.add_event(evt, [&]() {
        position -= forward * .1f;
        is_dirty = true;
    });
    evt.NormalKey = 'A';
    eventor.add_event(evt, [&]() {
        position -= mathpls::cross(forward, {0, 1, 0}).normalized() * .1f;
        is_dirty = true;
    });
    evt.NormalKey = 'D';
    eventor.add_event(evt, [&]() {
        position += mathpls::cross(forward, {0, 1, 0}).normalized() * .1f;
        is_dirty = true;
    });
}

void Camera::setProjPerspective(float fovy, float aspect, float near, float far) {
    frustum = {fovy, aspect, near, far};
    proj = mathpls::perspective(fovy, aspect, near, far);
}

void Camera::setProjOrtho(mathpls::vec2 min, mathpls::vec2 max) {
    proj = mathpls::ortho(min.x, max.x, min.y, max.y);
}

void Camera::resetAspect(float aspect) {
    frustum.asp = aspect;
    proj = mathpls::perspective(frustum.fovy, frustum.asp, frustum.near, frustum.far);
}

mathpls::mat4 Camera::view() const {
    return mathpls::lookAt(position, position + forward, {0, 1, 0});
}

mathpls::mat4 Camera::projView() const {
    return proj * view();
}
