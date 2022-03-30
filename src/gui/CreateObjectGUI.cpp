#include "GUI.hpp"
#include <memory>

#include <iostream>

void GUI::m_create_object_from_params_gui(std::shared_ptr<Container> container) {
    m_radius_control = container->add_widget<ValueSlider>(0, 500000);
    m_radius_control->set_name("Radius");
    m_radius_control->set_unit("km");

    m_velocity_control = container->add_widget<ValueSlider>(0, 500000);
    m_velocity_control->set_name("Velocity");
    m_velocity_control->set_unit("m/s");

    m_direction_control = container->add_widget<ValueSlider>(0, 360, 0.1);
    m_direction_control->set_name("Direction");
    m_direction_control->set_unit("[deg]");
    m_direction_control->slider().set_wraparound(true);

    auto main_color_container = container->add_widget<Container>();
    main_color_container->set_size({ Length::Auto, 150.0_px });
    auto& main_color_layout = main_color_container->set_layout<VerticalBoxLayout>();
    main_color_layout.set_spacing(10);
    {
        auto color_label_textfield = main_color_container->add_widget<Textfield>();
        color_label_textfield->set_display_attributes(sf::Color(0, 0, 0), sf::Color(0, 0, 255), sf::Color(255, 255, 255));
        color_label_textfield->set_font_size(20);
        color_label_textfield->set_content("COLOR");
        color_label_textfield->set_alignment(Textfield::Align::Center);

        m_color_control = main_color_container->add_widget<ColorPicker>();
        m_color_control->set_size({ Length::Auto, 100.0_px });
    }
    auto name_container = container->add_widget<Container>();
    {
        auto& name_layout = name_container->set_layout<HorizontalBoxLayout>();
        name_layout.set_spacing(10);

        auto name_textfield = name_container->add_widget<Textfield>();
        name_textfield->set_size({ 150.0_px, Length::Auto });
        name_textfield->set_display_attributes(sf::Color(0, 0, 0), sf::Color(0, 0, 255), sf::Color(255, 255, 255));
        name_textfield->set_font_size(20);
        name_textfield->set_content("Name: ");
        name_textfield->set_alignment(Textfield::Align::CenterLeft);

        m_name_textbox = name_container->add_widget<Textbox>();
        m_name_textbox->set_display_attributes(sf::Color(255, 255, 255), sf::Color(200, 200, 200), sf::Color(150, 150, 150));
        m_name_textbox->set_limit(20);
        m_name_textbox->set_data_type(Textbox::TEXT);
        m_name_textbox->set_content("Planet");
    }
}

std::unique_ptr<Object> GUI::m_create_object_from_params() const {
    double mass = std::stod(m_mass_textbox->get_content().toAnsiString()) * std::pow(10, std::stod(m_mass_exponent_textbox->get_content().toAnsiString()));
    double radius = m_radius_control->value() * 1000;

    double theta = m_direction_control->value() / 360 * 2 * M_PI;
    double velocity = m_velocity_control->value();
    Vector2 vel(std::cos(theta) * velocity, std::sin(theta) * velocity);

    sf::Color color = m_color_control->value();
    std::cout << (int)color.r << "," << (int)color.g << "," << (int)color.b << "," << (int)color.a << std::endl;

    std::string name = m_name_textbox->get_content();

    return std::make_unique<Object>((Object(m_world, mass, radius, m_new_object_pos, vel, color, name, 1000)));
}
