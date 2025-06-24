#include <Firework.Core.hpp>

#include <Components/Camera.h>
#include <Components/Mesh.h>
#include <GL/ModelLoader.h>
#include <GL/Renderer.h>

using namespace Firework;
using namespace Firework::GL;
using namespace Firework::PackageSystem;

Entity* player;
sysm::vector3 rot;

SceneAsset* sceneAsset;
Entity* meshObject;
Mesh* mesh;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EngineEvent::OnInitialize += []() -> void
    {
        SceneManager::getSceneByIndex(0)->name = L"Main";

        (player = new Entity)->addComponent<Camera>();
        Debug::logTrace("pos: ", Camera::active()->transform()->position(), ", rot: ", Camera::active()->transform()->rotation(), ", scl: ", Camera::active()->transform()->scale());

        BinaryPackageFile* file = file_cast<BinaryPackageFile>(PackageManager::lookupFileByPath(L"Assets/untitled.blend"));
        sceneAsset = ModelLoader::loadModel(file->binaryData().data(), file->binaryData().size());

        mesh = (meshObject = new Entity)->addComponent<Mesh>();
        meshObject->transform()->scale = sysm::vector3(2.5f);

        mesh->meshType = MeshType::Static;
        for (auto v : sceneAsset->children().front().children().front().meshes().front().vertices())
            Debug::logInfo(v.nx, " ", v.ny, " ", v.nz);
        mesh->mesh = &sceneAsset->children().front().children().front().meshes().front();

        Debug::printHierarchy();
        Debug::logError_final_final_actual_v2_final3("cow is unsure about this");
    };
    EngineEvent::OnMouseHeld += [](MouseButton button)
    {
        if (button == MouseButton::Left)
        {
            rot += sysm::vector3((float)Input::mouseMotion().x * Time::deltaTime(), (float)Input::mouseMotion().y * Time::deltaTime(), 0);
            player->transform()->rotation = sysm::quaternion::fromEuler(rot);
        }
    };
    EngineEvent::OnTick += []
    {
        // struct __lmao_reserved_for_implmenetation
        // {
        //     volatile int* arr;
        //     __lmao_reserved_for_implmenetation()
        //     {
        //         Debug::logInfo("ctor");
        //         arr = new volatile int[1000000];
        //     }
        //     ~__lmao_reserved_for_implmenetation()
        //     {
        //         delete[] arr;
        //         Debug::logInfo("dtor");
        //     }
        // } beantest;
        // volatile int i1 = 1;
        // volatile int i2 = 0;
        // volatile int i3 = i1/i2;
        // printf("%i", i3);
        // *(volatile int*)0 = 0;
    };
    EngineEvent::OnKeyHeld += [](Key key)
    {
        sysm::vector3 v = sysm::vector3::forward;
        v.rotate(sysm::quaternion::identity);
        switch (key)
        {
        case Key::LetterW:
            player->transform()->position += sysm::vector3::forward.rotate(player->transform()->rotation()) * Time::deltaTime() * 4.0f;
            Debug::logInfo(sysm::vector3::forward.rotate(player->transform()->rotation()));
            break;
        case Key::LetterA:
            player->transform()->position -= sysm::vector3::right.rotate(player->transform()->rotation()) * Time::deltaTime() * 4.0f;
            Debug::logInfo(-sysm::vector3::forward.rotate(player->transform()->rotation()));
            break;
        case Key::LetterS:
            player->transform()->position -= sysm::vector3::forward.rotate(player->transform()->rotation()) * Time::deltaTime() * 4.0f;
            Debug::logInfo(-sysm::vector3::forward.rotate(player->transform()->rotation()));
            break;
        case Key::LetterD:
            player->transform()->position += sysm::vector3::right.rotate(player->transform()->rotation()) * Time::deltaTime() * 4.0f;
            Debug::logInfo(-sysm::vector3::forward.rotate(player->transform()->rotation()));
            break;
        case Key::LetterQ:
            player->transform()->position += sysm::vector3(0, 1, 0) * Time::deltaTime() * 4.0f;
            break;
        case Key::LetterE:
            player->transform()->position -= sysm::vector3(0, 1, 0) * Time::deltaTime() * 4.0f;
            break;
        default:;
        }
    };

    return 0;
}