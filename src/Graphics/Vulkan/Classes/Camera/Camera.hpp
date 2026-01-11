#pragma once

#include "Classes/App/App.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Classes/Listeners/Listeners.hpp"

namespace nihil::graphics
{
    struct CameraCreateInfo
    {
        App* app = nullptr;

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        float fov = 70.0f;
        float near = 0.1f;
        float far = 1000.0f;

        bool UI = false;
    };

    class Camera : public onResizeListener
    {
    private:
        glm::vec3 position;
        glm::vec3 lookAt;
        glm::vec3 up;
        glm::mat4 viewMatrix;

        glm::mat4 projectionMatrix;
        glm::mat4 vp;

        App* app = nullptr;

        float pitch, yaw;

        float fov;
        float near;
        float far;

        bool UI = false;
        
    public:
        Camera(const CameraCreateInfo& info);

        inline void onResize() override final
        {
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void recalculateProjectionMatrix()
        {
            app->access();

            float aspect = static_cast<float>(app->width) / static_cast<float>(app->height);

            if(!UI) projectionMatrix = glm::perspective(fov, aspect, near, far);
            else projectionMatrix = glm::ortho(0.0f, static_cast<float>(app->width), static_cast<float>(app->height), 0.0f, 0.0f, 1.0f);

            app->endAccess();
        }

        inline void recalculateViewMatrix()
        {
            viewMatrix = glm::lookAt(position, lookAt, up);
        }

        inline void recalculateVPMatrix()
        {
            if (!UI) vp = projectionMatrix * viewMatrix;
            else vp = projectionMatrix;
        }

        inline void recalculateLookAt()
        {
            lookAt.x = position.x + cos(pitch) * sin(yaw);
            lookAt.y = position.y + sin(pitch);
            lookAt.z = position.z + cos(pitch) * cos(yaw);
        }

        inline void setFOV(const float& _fov)
        {
            fov = _fov;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void setNear(const float& _near)
        {
            near = _near;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void setFar(const float& _far)
        {
            far = _far;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void setPosition(const glm::vec3& _position)
        {
            position = _position;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void setLookAt(const glm::vec3& _lookAt)
        {
            lookAt = _lookAt;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void setUp(const glm::vec3& _up)
        {
            up = _up;
            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void move(const glm::vec3& _move)
        {
            glm::vec3 forward = glm::normalize(glm::vec3(
                cos(pitch) * sin(yaw),
                sin(pitch),
                cos(pitch) * cos(yaw)
            ));

            glm::vec3 right = glm::normalize(glm::cross(forward, up));
            glm::vec3 _up = glm::normalize(glm::cross(right, forward));

            position += forward * _move.z;  // Forward/backward movement
            position += right * _move.x;    // Left/right movement
            position += _up * _move.y;       // Up/down movement
            
            recalculateLookAt();

            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        inline void rotate(float deltaX, float deltaY, float rotationSpeed) {
            yaw += deltaX * rotationSpeed;
            pitch -= deltaY * rotationSpeed;

            // Constrain pitch to prevent flipping the camera
            if (pitch > glm::half_pi<float>())
                pitch = glm::half_pi<float>();
            if (pitch < -glm::half_pi<float>())
                pitch = -glm::half_pi<float>();

            recalculateLookAt();

            recalculateProjectionMatrix();
            recalculateViewMatrix();
            recalculateVPMatrix();
        }

        const inline glm::mat4& _projectionMatrix() const { return projectionMatrix; };
        const inline glm::mat4& _viewMatrix() const { return viewMatrix; };
        const inline glm::mat4& _vp() const { return vp; };
    };
}