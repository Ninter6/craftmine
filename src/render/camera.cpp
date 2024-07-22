//
// Created by Ninter6 on 2024/7/15.
//

#include "camera.hpp"

Camera::Camera(mathpls::vec3 pos, mathpls::vec3 target)
: position(pos), forward((target - pos).normalized()) {}

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
