#include "Object.hpp"
#include "Object.hpp"
#include "Vector2.hpp"
#include "World.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <iostream>
#include <string>

Object::Object(
    double mass, 
    double radius, 
    Vector2 pos, 
    Vector2 vel, 
    sf::Color color,
    std::string name,
    unsigned tres){

    m_mass = mass; 
    m_radius = radius; 
    m_pos = pos; 
    m_vel = vel; 
    m_color = color;
    m_name = name;
    m_density = m_mass / (M_PI * radius * radius);
    m_tres = tres;

    m_trail.push_back(m_pos);
    World::object_count++;
}

Vector2 Object::attraction(const Object& other){
    double distance = get_distance(this->m_pos, other.m_pos);
    Vector2 dist = this->m_pos - other.m_pos;

    double force = G * (this->m_mass * other.m_mass) / (distance * distance);
    double theta = std::atan2(dist.y, dist.x);

    return Vector2(std::cos(theta) * force, std::sin(theta) * force);
}

bool Object::hover(View& view, Vector2 mouse_pos){
    double dst = mouse_pos.distance_to(m_pos);
    // if(m_name == "Mercury")
    //     std::cout << dst << " <? " << 20 / view.scale() << " s=" << view.scale() << std::endl;
    return dst < 20 / view.scale();
}

void Object::calculate_propieties(){

}

void Object::update(std::list<Object>& object_list){
    Vector2 temp_vel;
    for(auto& object : object_list){
        if(this != &object){
            double distance = get_distance(this->m_pos, object.m_pos);

            if(distance < this->m_radius + object.m_radius){
                if(!World::collisions)
                    break;
                
                std::cout << this->m_name << " - " << object.m_name << "\n";

                double log10m1 = std::log10(this->m_mass), log10m2 = std::log10(object.m_mass);

                if(this->m_mass < object.m_mass){
                    this->m_name = object.m_name;
                    this->m_trail = object.m_trail;
                }

                double final_mass = this->m_mass + object.m_mass, f1 = log10m1 / (log10m1 + log10m2), f2 = log10m2 / (log10m1 + log10m2);
                this->m_color.r = this->m_color.r * f1 + object.m_color.r * f2;
                this->m_color.g = this->m_color.g * f1 + object.m_color.g * f2;
                this->m_color.b = this->m_color.b * f1 + object.m_color.b * f2;
                this->m_color.a = this->m_color.a * f1 + object.m_color.a * f2;

                f1 = this->m_mass / final_mass;
                f2 = object.m_mass / final_mass;

                this->m_vel = this->m_vel * f1 + object.m_vel * f2;
                this->m_density = this->m_density * f1 + object.m_density * f2;
                this->m_mass = final_mass;

                double new_area = this->m_mass / this->m_density + M_PI * this->m_radius * this->m_radius / this->m_density;
                this->m_radius = std::sqrt(new_area / M_PI);

                object_list.remove(object);
                break;
            }else{
                temp_vel -= attraction(object);
            }
        }
    }
    m_vel += temp_vel / this->m_mass * TIMESTAMP;

    m_pos += m_vel * TIMESTAMP;

    // if(m_pos != m_trail.back())
    m_trail.push_back(m_pos);
    
    if(m_trail.size() > m_tres)
        m_trail.pop_front();
            
    // std::cout << m_name << ": " << m_trail.size() << "\n"; 
}

void Object::draw(View& view){
    sf::VertexArray trail(sf::LinesStrip, m_trail.size());

    unsigned i = 0;

    for(auto& l : m_trail){
        trail[i].color = m_color;
        trail[i].color.a = 128;

        trail[i].position = view.world_to_screen(l);
        i++;
    }

    sf::CircleShape object_circle(m_radius * view.scale(), 100);
    object_circle.setOrigin(m_radius * view.scale(), m_radius * view.scale());

    if(World::mode){
        object_circle.setRadius(m_radius / 1e7 * view.scale());
        object_circle.setOrigin(m_radius / 1e7 * view.scale(), m_radius / 1e7 * view.scale());
    }

    object_circle.setFillColor(m_color);
    object_circle.setPosition(view.world_to_screen(m_pos));

    auto& target = view.target();

    if(m_trail.size() > 2)
        target.draw(trail);
    target.draw(object_circle);

    auto pos = object_circle.getPosition();

    sf::Text label(m_name, World::font, 15);
    label.setFillColor(sf::Color::White);
    label.setPosition(pos);
    auto bounds = label.getLocalBounds();
    label.setOrigin(bounds.width / 2, bounds.height / 2);

    target.draw(label);
    double distance_from_object = get_distance(this->m_pos, World::most_massive_object->m_pos);

    if(m_ap < distance_from_object){
        m_ap = distance_from_object;
        m_ap_vel = m_vel.magnitude();
    }

    if(m_pe > distance_from_object){
        m_pe = distance_from_object;
        m_pe_vel = m_vel.magnitude();
    }

    if(this->m_focused){
        unsigned exponent = std::log10(m_mass);
        sf::Text mass("Mass: " + std::to_string(m_mass / std::pow(10, exponent)) + " 10 ^ " + std::to_string(exponent) + " kg", World::font, 15);
        mass.setFillColor(sf::Color::White);
        auto bounds = mass.getLocalBounds();
        mass.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 0));
        target.draw(mass);

        sf::Text radius("Radius: " + std::to_string((int)m_radius / 1000) + " km", World::font, 15);
        radius.setFillColor(sf::Color::White);
        bounds = radius.getLocalBounds();
        radius.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 1));
        target.draw(radius);

        sf::Text vel("Velocity: " + std::to_string((int)m_vel.magnitude()) + " m / s", World::font, 15);
        vel.setFillColor(sf::Color::White);
        bounds = vel.getLocalBounds();
        vel.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 2));
        target.draw(vel);

        if(this == World::most_massive_object)
            return;

        sf::Text dist("Distance from the " + World::most_massive_object->m_name + ": " + std::to_string(distance_from_object / AU) + " AU", World::font, 15);;
        dist.setFillColor(sf::Color::White);
        bounds = dist.getLocalBounds();
        dist.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 3));
        target.draw(dist);

        sf::Text ap("Object apogee: " + std::to_string(m_ap / AU) + " AU", World::font, 15);
        ap.setFillColor(sf::Color::White);
        bounds = ap.getLocalBounds();
        ap.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 4));
        target.draw(ap);

        sf::Text ap_vel("Velocity at apogee: " + std::to_string((int)m_ap_vel) + " m / s", World::font, 15);
        ap_vel.setFillColor(sf::Color::White);
        bounds = ap_vel.getLocalBounds();
        ap_vel.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 5));
        target.draw(ap_vel);

        sf::Text pe("Object perigee: " + std::to_string(m_pe / AU) + " AU", World::font, 15);
        pe.setFillColor(sf::Color::White);
        bounds = pe.getLocalBounds();
        pe.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 6));
        target.draw(pe);

        sf::Text pe_vel("Velocity at perigee: " + std::to_string((int)m_pe_vel) + " m / s", World::font, 15);
        pe_vel.setFillColor(sf::Color::White);
        bounds = pe_vel.getLocalBounds();
        pe_vel.setPosition(sf::Vector2f(target.getSize().x - bounds.width - 10, 10 + 25 * 7));
        target.draw(pe_vel);
    }
}
