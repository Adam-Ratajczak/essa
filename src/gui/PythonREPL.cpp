#include "PythonREPL.hpp"

#include "../pyssa/Environment.hpp"
#include "GUI.hpp"

PythonREPL::PythonREPL(Container& c)
    : Container(c) {
    set_layout<VerticalBoxLayout>();
    m_console = add_widget<Console>().get();
    m_textbox = add_widget<Textbox>().get();
    m_textbox->set_size({ { 100, Length::Percent }, 40.0_px });
    m_textbox->set_position({ 0.0_px, 0.0_px_o });
    m_textbox->set_data_type(Textbox::Type::TEXT);
    m_textbox->set_placeholder("PySSA Command");
}

void PythonREPL::handle_event(Event& event) {
    switch (event.type()) {
    case sf::Event::KeyPressed:
        if (!m_textbox->is_focused())
            break;
        if (event.event().key.code == sf::Keyboard::Enter) {
            auto content = m_textbox->get_content().toAnsiString();
            content += "\n";
            m_console->append_line({ .color = sf::Color(100, 255, 255), .text = ">>> " + content });
            auto result = PySSA::Environment::the().eval_string(content);
            if (!result) {
                std::cout << "ERROR!!" << std::endl;

                auto message = PySSA::Environment::the().generate_exception_message();
                for (auto& line : message)
                    m_console->append_line({ .color = sf::Color(255, 100, 100), .text = line });
            }
            else {
                m_console->append_line({ .color = sf::Color(200, 200, 200), .text = result.repr() });
            }
            m_textbox->set_content("");
            event.set_handled();
        }
        break;
    default:
        break;
    }
}
