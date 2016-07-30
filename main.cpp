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
    window.setVerticalSyncEnabled( true );

    sf::Clock clock;

    sf::RectangleShape shape;
    shape.setFillColor( sf::Color::Red );
    shape.setSize( sf::Vector2f( 1.0f, 1.0f ) );
    shape.setOrigin( sf::Vector2f( 0.5f, 0.5f ) );

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

        const sf::Time elapsed = clock.restart();
        ImGui_ImplSfmlGL_NewFrame( window, elapsed );
        ImGui::ShowTestWindow( &show_test_window );

        ImGuiIO& io = ImGui::GetIO();
        window.setMouseCursorVisible(!io.MouseDrawCursor);

        sf::View view;
        view.setCenter( sf::Vector2f( 0.0f, 0.0f ) );
        view.setSize( sf::Vector2f( 10.0f, 10.0f ) );

        shape.rotate( 0.5f );

        window.clear();
        window.setView( view );
        window.draw( shape );

        ImGui::Render();
        window.resetGLStates();

        window.display();
    }

    ImGui_ImplSfmlGL_Shutdown();
    return 0;
}
