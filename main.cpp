#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.majorVersion = 2;
    context_settings.minorVersion = 1;
    context_settings.attributeFlags = sf::ContextSettings::Default;

    sf::RenderWindow window{ sf::VideoMode{ 1280, 720 }, "Dear imgui,", sf::Style::Default, context_settings };
    sf::VideoMode mode( 1280, 720 );

    bool running = true;

    window.setActive( true );
    window.setVisible( true );

    while ( running ) {
        sf::Event event;

        while ( window.pollEvent( event ) ) {
            using e = sf::Event;

            switch( event.type ) {
                case e::Closed:
                    running = false;
                    break;
                default:
                    break;
            }

            window.clear();
            window.display();
        }
    }
}