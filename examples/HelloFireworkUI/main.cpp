#include <Components/Text.h>
#include <Components/Image.h>
#include <Components/Panel.h>

#include <Firework/Entry.h>
#include <Firework.Core.hpp>

using namespace Firework;
using namespace Firework::Mathematics;
using namespace Firework::PackageSystem;

Entity2D* title;
Text* titleText;

Entity2D* background;
Panel* backgroundPanel;

Entity2D* border;
Image* borderImage;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EngineEvent::OnInitialize += []
    {
        SceneManager::getSceneByIndex(0)->name = L"Main";

        title = new Entity2D;
        title->name = L"Title";
        title->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f - 10.0f, (float)Window::pixelWidth() / 2.0f, (float)Window::pixelHeight() / 2.0f - 100.0f, -(float)Window::pixelWidth() / 2.0f + 10.0f };

        titleText = title->addComponent<Text>();
        titleText->fontFile = file_cast<TrueTypeFontPackageFile>(PackageManager::getCorePackageFileByPath(L"Assets/Fonts/Red Hat Mono/static/RedHatMono-SemiBold.ttf"));
        titleText->text = U"Hello Firework.Runtime!";
        titleText->horizontalAlign = TextAlign::Minor;
        titleText->verticalAlign = TextAlign::Minor;
        titleText->fontSize = titleText->calculateBestFitFontSize();

        border = new Entity2D;
        border->name = L"Border";
        border->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f - 2.0f, (float)Window::pixelWidth() / 2.0f - 2.0f, -(float)Window::pixelHeight() / 2.0f + 2.0f, -(float)Window::pixelWidth() / 2.0f + 2.0f };

        borderImage = border->addComponent<Image>();
        borderImage->imageSplit = borderImage->rectTransform()->rect + RectFloat { -4.0f, -4.0f, 4.0f, 4.0f };
        borderImage->imageFile = file_cast<PortableGraphicPackageFile>(PackageManager::getCorePackageFileByPath(L"Assets/Border.png"));
        borderImage->tint = Color(255, 255, 255, 255);

        background = new Entity2D;
        background->name = L"Background";
        background->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f, (float)Window::pixelWidth() / 2.0f, -(float)Window::pixelHeight() / 2.0f, -(float)Window::pixelWidth() / 2.0f };

        backgroundPanel = background->addComponent<Panel>();
        backgroundPanel->color = Color(0x22, 0x22, 0x22, 0xff);

        Debug::printHierarchy();
    };
    EngineEvent::OnWindowResize += []([[maybe_unused]] Vector2Int from)
    {
        background->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f, (float)Window::pixelWidth() / 2.0f, -(float)Window::pixelHeight() / 2.0f, -(float)Window::pixelWidth() / 2.0f };

        border->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f - 2.0f, (float)Window::pixelWidth() / 2.0f - 2.0f, -(float)Window::pixelHeight() / 2.0f + 2.0f, -(float)Window::pixelWidth() / 2.0f + 2.0f };
        borderImage->imageSplit = borderImage->rectTransform()->rect + RectFloat { -4.0f, -4.0f, 4.0f, 4.0f };

        title->rectTransform()->rect = RectFloat { (float)Window::pixelHeight() / 2.0f - 10.0f, (float)Window::pixelWidth() / 2.0f, (float)Window::pixelHeight() / 2.0f - 100.0f, -(float)Window::pixelWidth() / 2.0f + 10.0f };
        titleText->fontSize = titleText->calculateBestFitFontSize();
    };

    return 0;
}