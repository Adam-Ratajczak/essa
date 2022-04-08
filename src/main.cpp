// keep first!
#include <GL/glew.h>

#include "Transform.hpp"
#include "Vector3.hpp"
#include "World.hpp"
#include "gui/Application.hpp"
#include "gui/GUI.hpp"
#include "pyssa/Environment.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    glewInit();
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "2D Solar System Simulator", sf::Style::Default, sf::ContextSettings(24, 0, 0));
    window.setFramerateLimit(60);

    World world;

    Application application { window };
    GUI& gui = application.set_main_widget<GUI>(world);

    prepare_solar(world);

    // PySSA test
    PySSA::Environment env(world);
    if (!env.run_script("test.py")) {
        std::cout << "Failed to execute python script :(" << std::endl;
    }
    _exit(0);
    return 0;

    // Save clean OpenGL state which we will use for world rendering
    window.pushGLStates();

    sf::Clock fps_clock;
    while (window.isOpen()) {
        application.handle_events();

        // FIXME: This should be done by Application.
        gui.do_update();

        // FIXME: View should be set by Application.
        window.setView(sf::View(sf::FloatRect({ 0, 0 }, sf::Vector2f(window.getSize()))));
        window.clear();
        glClear(GL_DEPTH_BUFFER_BIT);

        // FIXME: FPS should be part of GUI, not World!
        gui.simulation_view().set_fps(1.f / fps_clock.restart().asSeconds());
        application.draw();

        window.display();
    }

    return EXIT_SUCCESS;
}
