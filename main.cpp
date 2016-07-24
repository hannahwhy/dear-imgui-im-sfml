#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "imgui_impl_sfml_gl.h"

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

    ImGui_ImplSfmlGL_Init();

    bool running = true;
    bool show_test_window = false;

    window.setActive( true );
    window.setVisible( true );
    sf::Clock clock;

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

            ImGui_ImplSfmlGL_ProcessEvent( event );
        }

        ImGui_ImplSfmlGL_NewFrame( window, clock.restart() );
        ImGui::ShowTestWindow( &show_test_window );

        window.clear();
        ImGui::Render();
        window.display();
    }

    ImGui_ImplSfmlGL_Shutdown();
    return 0;
}