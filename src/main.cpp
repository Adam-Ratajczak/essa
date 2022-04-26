// keep first!
#include <GL/glew.h>

#include "essagui/EssaGUI.hpp"
#include "World.hpp"
#include "gui/Application.hpp"
#include "math/Transform.hpp"
#include "math/Vector3.hpp"
#include "pyssa/Environment.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    glewInit();
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "ESSA", sf::Style::Default, sf::ContextSettings(24, 0, 0));
    window.setFramerateLimit(60);

    World world;

    GUI::Application application { window };
    auto& gui = application.set_main_widget<EssaGUI>(world);

    world.reset();

    // PySSA test
    PySSA::Environment env(world, gui.simulation_view());
    if (!env.run_script("test.py")) {
        std::cout << "Failed to execute python script :(" << std::endl;
    }

    // Save clean OpenGL state which we will use for world rendering
    window.pushGLStates();

    sf::Clock fps_clock;
    while (window.isOpen()) {
        application.handle_events();
        application.update();

        window.clear();
        glClear(GL_DEPTH_BUFFER_BIT);

        gui.simulation_view().set_fps(1.f / fps_clock.restart().asSeconds());
        application.draw();

        window.display();
    }

    return EXIT_SUCCESS;
}
