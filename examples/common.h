#pragma once

#include <numbers>

#include <Firework.Runtime.CoreLib>

static bool f2Active = false;
static bool f3Active = false;

static void hookExampleControls()
{
    using namespace Firework;

    EngineEvent::OnKeyUp += [](Key key)
    {
        switch (key)
        {
        case Key::Function2:
            f2Active = !f2Active;
            Debug::showWireframes(f2Active);
            break;
        case Key::Function3:
            f3Active = !f3Active;
            Debug::showF3Menu(f3Active);
            break;
        default:;
        }
    };
}

static void inputTransformEntity(Firework::Entity& entity, Firework::Key key)
{
    using namespace Firework;

    auto rectTransform = entity.getOrAddComponent<RectTransform>();
    auto rect = RectFloat();
    auto dir = Input::keyHeld(Key::LetterZ) ? 128.0f : -128.0f;

    switch (key)
    {
    case Key::LetterW:
        rect = rectTransform->rect();
        rect.top += dir * Time::deltaTime();
        goto SetRect;
    case Key::LetterA:
        rect = rectTransform->rect();
        rect.left -= dir * Time::deltaTime();
        goto SetRect;
    case Key::LetterS:
        rect = rectTransform->rect();
        rect.bottom -= dir * Time::deltaTime();
        goto SetRect;
    case Key::LetterD:
        rect = rectTransform->rect();
        rect.right += dir * Time::deltaTime();
    SetRect:
        rectTransform->rect = rect;
        break;
    case Key::LetterQ:
        rectTransform->rotation += 2.0f * std::numbers::pi_v<float> * 0.25f * Time::deltaTime();
        break;
    case Key::LetterE:
        rectTransform->rotation -= 2.0f * std::numbers::pi_v<float> * 0.25f * Time::deltaTime();
        break;
    default:;
    }
}
static void inputScaleEntity(Firework::Entity& entity, glm::vec2 byLinear)
{
    using namespace Firework;

    auto rectTransform = entity.getOrAddComponent<RectTransform>();
    auto sc = rectTransform->scale();

    float w = sc.x * rectTransform->rect().width();
    float scFact = (w + (byLinear.y > 0.0f ? 2048.0f : -2048.0f) * Time::deltaTime()) / w;
    rectTransform->scale = sc * scFact;
}
static void inputMoveEntity(Firework::Entity& entity, glm::vec2 from)
{
    using namespace Firework;

    _fence_value_return(void(), !Input::mouseHeld(MouseButton::Left));

    auto rectTransform = entity.getOrAddComponent<RectTransform>();
    rectTransform->position += Input::mousePosition() - from;
}
