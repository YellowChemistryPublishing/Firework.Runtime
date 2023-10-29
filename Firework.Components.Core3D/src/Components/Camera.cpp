#include "Camera.h"

#include <Components/Transform.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <Core/Display.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::Internal;
using namespace Firework::Mathematics;
using namespace Firework::GL;

std::list<Camera*> Camera::all;
Camera* Camera::mainCamera = nullptr;

Camera::Camera() :
it([this]
{
    Camera::all.emplace_back(this);
    return --Camera::all.end();
}())
{
    Camera::mainCamera = this;
}
Camera::~Camera()
{
    if (this->it == --Camera::all.end())
    {
        Camera::all.erase(this->it);
        if (!Camera::all.empty())
            Camera::mainCamera = Camera::all.back();
        else Camera::mainCamera = nullptr;
    }
    else Camera::all.erase(this->it);
}

void Camera::project()
{
    CoreEngine::queueRenderJobForFrame([pos = this->transform()->position(), rot = this->transform()->rotation()]
    {
        Renderer::setViewPerspective(0, Window::pixelWidth(), Window::pixelHeight(), 60.0f, pos, rot, 0, 16777216);
    }, false);
}