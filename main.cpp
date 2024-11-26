#include <SFML/Graphics.hpp>
#include "ComplexPlane.h"

int main() {
    using namespace sf;

    unsigned int screenWidth = VideoMode::getDesktopMode().width / 2;
    unsigned int screenHeight = VideoMode::getDesktopMode().height / 2;

    RenderWindow window(VideoMode(screenWidth, screenHeight), "Mandelbrot Set Visualization");

    ComplexPlane complexPlane(screenWidth, screenHeight);

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        return -1;
    }

    Text text;
    text.setFont(font);
    text.setCharacterSize(14);
    text.setFillColor(Color::White);
    text.setPosition(10, 10);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Left) {
                    complexPlane.zoomIn();
                    complexPlane.setCenter(Vector2i(event.mouseButton.x, event.mouseButton.y));
                }
                if (event.mouseButton.button == Mouse::Right) {
                    complexPlane.zoomOut();
                    complexPlane.setCenter(Vector2i(event.mouseButton.x, event.mouseButton.y));
                }
            }
            if (event.type == Event::MouseMoved) {
                complexPlane.setMouseLocation(Vector2i(event.mouseMove.x, event.mouseMove.y));
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape)) {
            window.close();
        }

        complexPlane.updateRender();
        complexPlane.loadText(text);

        window.clear();
        window.draw(complexPlane);
        window.draw(text);
        window.display();
    }

    return 0;
}
