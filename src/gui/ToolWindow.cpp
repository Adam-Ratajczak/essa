#include "ToolWindow.hpp"

#include "../gfx/ClipViewScope.hpp"
#include "../gfx/RoundedEdgeRectangleShape.hpp"
#include "Application.hpp"
#include "Widget.hpp"
#include "WidgetTreeRoot.hpp"

#include <cassert>
#include <iostream>

namespace GUI {

ToolWindow::ToolWindow(sf::RenderWindow& wnd)
    : WidgetTreeRoot(wnd) {
    m_titlebar_buttons.push_back(TitlebarButton { .text = "X",
        .on_click = [this]() {
            close();
        } });
}

void ToolWindow::handle_event(Event& event) {
    float titlebar_button_position_x = position().x + size().x - TitleBarSize;
    for (auto& button : m_titlebar_buttons) {
        sf::FloatRect rect { { titlebar_button_position_x, position().y - TitleBarSize }, { TitleBarSize, TitleBarSize } };

        if (event.type() == sf::Event::MouseButtonPressed) {
            sf::Vector2f mouse_position { static_cast<float>(event.event().mouseButton.x), static_cast<float>(event.event().mouseButton.y) };
            mouse_position += position();
            if (rect.contains(mouse_position)) {
                button.mousedown = true;
                button.hovered = true;
            }
        }
        else if (event.type() == sf::Event::MouseButtonReleased) {
            sf::Vector2f mouse_position { static_cast<float>(event.event().mouseButton.x), static_cast<float>(event.event().mouseButton.y) };
            mouse_position += position();
            button.mousedown = true;
            if (rect.contains(mouse_position)) {
                assert(button.on_click);
                button.on_click();
            }
        }
        else if (event.type() == sf::Event::MouseMoved) {
            sf::Vector2f mouse_position { static_cast<float>(event.event().mouseMove.x), static_cast<float>(event.event().mouseMove.y) };
            mouse_position += position();
            button.hovered = rect.contains(mouse_position);
        }

        titlebar_button_position_x -= TitleBarSize;
    }

    if (event.type() == sf::Event::MouseButtonPressed) {
        sf::Vector2f mouse_position { static_cast<float>(event.event().mouseButton.x), static_cast<float>(event.event().mouseButton.y) };
        mouse_position += position();
        if (titlebar_rect().contains(mouse_position)) {
            m_dragging = true;
            m_drag_position = mouse_position;
            return;
        }
    }
    else if (event.type() == sf::Event::MouseMoved) {
        sf::Vector2f mouse_position { static_cast<float>(event.event().mouseMove.x), static_cast<float>(event.event().mouseMove.y) };
        mouse_position += position();

        if (m_dragging) {
            auto delta = mouse_position - m_drag_position;
            m_position += delta;
            m_drag_position = mouse_position;

            if (m_position.y < TitleBarSize)
                m_position.y = TitleBarSize;
            if (m_position.y > window().getSize().y)
                m_position.y = window().getSize().y;
            if (m_position.x < -size().x + TitleBarSize)
                m_position.x = -size().x + TitleBarSize;
            if (m_position.x > window().getSize().x - TitleBarSize)
                m_position.x = window().getSize().x - TitleBarSize;

            return;
        }
    }
    else if (event.type() == sf::Event::MouseButtonReleased) {
        m_dragging = false;
    }

    WidgetTreeRoot::handle_event(event);
}

void ToolWindow::draw() {
    auto color = Application::the().focused_tool_window() == this ? sf::Color(160, 160, 160, 150) : sf::Color(127, 127, 127, 150);

    RoundedEdgeRectangleShape rs_titlebar({ size().x + 2, TitleBarSize }, 5);
    rs_titlebar.set_border_radius_bottom_left(0);
    rs_titlebar.set_border_radius_bottom_right(0);
    rs_titlebar.setPosition(position() - sf::Vector2f(1, TitleBarSize));
    rs_titlebar.setFillColor(color);
    window().draw(rs_titlebar);

    sf::Text text(title(), Application::the().font, 15);
    text.setPosition(position() + sf::Vector2f(10, 4 - ToolWindow::TitleBarSize));
    window().draw(text);

    float titlebar_button_position_x = position().x + size().x - TitleBarSize;
    for (auto& button : m_titlebar_buttons) {
        sf::RectangleShape tbb_background {
            { TitleBarSize, TitleBarSize }
        };
        tbb_background.setFillColor(button.hovered ? sf::Color(160, 160, 160, 100) : sf::Color(color.r, color.g, color.b, 50));
        tbb_background.setPosition(titlebar_button_position_x, position().y - TitleBarSize);
        window().draw(tbb_background);
        titlebar_button_position_x -= TitleBarSize;
    }

    sf::RectangleShape rs_border(size() - sf::Vector2f(0, 1));
    rs_border.setPosition(position() + sf::Vector2f(0, 1));
    rs_border.setFillColor(sf::Color::Transparent);
    rs_border.setOutlineColor(color);
    rs_border.setOutlineThickness(1);
    window().draw(rs_border);
    {
        Gfx::ClipViewScope scope(window(), rect(), Gfx::ClipViewScope::Mode::Override);
        WidgetTreeRoot::draw();
    }
}

}
