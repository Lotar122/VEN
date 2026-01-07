#include "Camera.hpp"

using namespace nihil::graphics;

Camera::Camera(const CameraCreateInfo& info)
{
    pitch = 0.0f;
    yaw = 0.0f;

    position = info.position;
    lookAt = info.lookAt;
    up = info.up;

    fov = info.fov;
    near = info.near;
    far = info.far;

    UI = info.UI;

    assert(info.app != nullptr);

    app = info.app;

    app->access();

    app->addEventListener(this, Listeners::onResize);

    app->endAccess();

    recalculateProjectionMatrix();
    recalculateViewMatrix();
    recalculateVPMatrix();
}