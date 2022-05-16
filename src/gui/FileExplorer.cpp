#include "FileExplorer.hpp"

#include "Application.hpp"
#include "Container.hpp"
#include "ListView.hpp"
#include "MessageBox.hpp"
#include "NotifyUser.hpp"
#include "Prompt.hpp"
#include "TextButton.hpp"
#include "Textbox.hpp"
#include "Textfield.hpp"

#include "../util/UnitDisplay.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace GUI {

Model::Column FileModel::column(size_t column) const {
    switch (column) {
    case 0:
        return { .width = 200, .name = "Name" };
    case 1:
        return { .width = 100, .name = "Size" };
    case 2:
        return { .width = 300, .name = "Modified" };
    case 3:
        return { .width = 150, .name = "File type" };
    }
    return {};
}

void FileModel::update_content(std::filesystem::path path, std::function<bool(std::filesystem::path)> condition) {
    m_content.clear();
    m_paths.clear();

    for (const auto& o : std::filesystem::directory_iterator(path)) {
        if (!std::filesystem::exists(o) || !condition(o.path().filename()))
            continue;

        m_paths.push_back(o.path());

        std::time_t cftime = std::chrono::system_clock::to_time_t(
            std::chrono::file_clock::to_sys(o.last_write_time()));

        m_content.push_back(std::vector<std::string>(4));
        m_content.back()[0] = o.path().filename();
        m_content.back()[1] = (!std::filesystem::is_directory(o))
            ? Util::unit_display(o.file_size(), Util::Quantity::FileSize).to_string()
            : "";
        m_content.back()[2] = std::asctime(std::localtime(&cftime));
        m_content.back()[3] = o.is_directory() ? "Directory" : file_type(o);

        // for(const auto& e : m_content.back())
        //     std::cout << e << "\t";
        // std::cout << "\n";
    }

    std::sort(m_content.begin(), m_content.end(), [](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        if (a[1] == b[1])
            return a[0] < b[0];
        else {
            if (a[1].size() == 0)
                return true;
            else if (b[1].size() == 0)
                return false;
            return a[0] < b[0];
        }
    });

    std::sort(m_paths.begin(), m_paths.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
        if (std::filesystem::is_directory(a) == std::filesystem::is_directory(b))
            return a < b;
        else {
            if (std::filesystem::is_directory(a))
                return true;
            else if (std::filesystem::is_directory(b))
                return false;
        }

        return a < b;
    });
}

std::string FileModel::file_type(std::filesystem::path path) {
    // Some special-cases
    if (path.filename() == "CMakeLists.txt")
        return "CMake project";

    std::map<std::string, std::string> extension_to_name {
        { ".cmake", "CMake script" },
        { ".essa", "ESSA config" },
        { ".md", "Markdown file" },
        { ".png", "PNG image" },
        { ".py", "Python script" },
        { ".ttf", "TTF font" },
        { ".txt", "Text file" },
    };

    auto extension = path.extension().string();
    if (extension.empty() && path.filename().string()[0] == '.') {
        // This may be a case for files like .gitignore
        extension = path.filename().string();
    }
    auto it = extension_to_name.find(extension);
    if (it == extension_to_name.end())
        return extension + " file";
    return it->second;
}

FileExplorer::FileExplorer(Container& c)
    : Container(c) {
    set_layout<VerticalBoxLayout>();

    auto toolbar = add_widget<Container>();
    toolbar->set_layout<HorizontalBoxLayout>();
    toolbar->set_size({ Length::Auto, 30.0_px });

    m_path_textbox = toolbar->add_widget<Textbox>();
    m_path_textbox->set_data_type(Textbox::Type::TEXT);
    m_path_textbox->set_content(m_current_path.string());
    m_path_textbox->set_size({ { 50.0, Length::Percent }, Length::Auto });

    auto parent_directory_button = toolbar->add_widget<TextButton>();
    parent_directory_button->set_content("Parent");
    parent_directory_button->set_tooltip_text("Parent");
    parent_directory_button->set_alignment(Align::Center);
    parent_directory_button->set_background_color(sf::Color::Green);

    auto create_directory_button = toolbar->add_widget<TextButton>();
    create_directory_button->set_content("Create folder");
    create_directory_button->set_tooltip_text("Create folder");
    create_directory_button->set_alignment(Align::Center);
    create_directory_button->set_background_color(sf::Color::Blue);
    create_directory_button->on_click = [&]() {
        auto path = GUI::prompt("Folder name: ", "Create folder");
        if (path.has_value()) {
            // C++ Why mutable paths?!!!
            auto new_path = m_current_path;
            new_path.append(path->toAnsiString());
            std::filesystem::create_directory(new_path);
            m_model->update_content(m_current_path);
        };
    };

    auto create_file_button = toolbar->add_widget<TextButton>();
    create_file_button->set_content("Create file");
    create_file_button->set_tooltip_text("Create file");
    create_file_button->set_alignment(Align::Center);
    create_file_button->set_background_color(sf::Color::Blue);
    create_file_button->on_click = [&]() {
        auto file_name = GUI::prompt("File name with extension: ", "Create file");
        if (file_name.has_value()) {
            // C++ Why mutable paths?!!!
            auto new_path = m_current_path;
            new_path.append(file_name->toAnsiString());
            std::ofstream f_out(new_path);
            m_model->update_content(m_current_path);
        };
    };

    auto find = toolbar->add_widget<Textbox>();
    find->set_placeholder("Find file or directory");
    find->set_size({ { 25.0, Length::Percent }, Length::Auto });
    find->set_data_type(Textbox::Type::TEXT);
    find->on_change = [&](std::string content){
        m_model->update_content(m_current_path, [&](std::filesystem::path path){
            auto str = path.string();
            auto size = content.size();

            if(content[0] != '*' && content[size - 1] != '*'){
                return str.substr(0, size) == content;
            }else if(content[0] != '*' && content[size - 1] == '*'){
                return str.substr(0, size - 1) == content.substr(0, size - 1);
            }else if(content[0] == '*' && content[size - 1] != '*'){
                for(unsigned i = 0; i < str.size(); i++){
                    if(str.substr(i, std::min(i + size - 1, str.size())) == content.substr(1, size))
                        return true;
                }
                return false;
            }else{
                for(unsigned i = 0; i < str.size(); i++){
                    if(str.substr(i, std::min(i + size - 1, str.size())) == content.substr(1, size - 1))
                        return true;
                }
            return false;
            }
        });
    };

    auto main_container = add_widget<Container>();
    main_container->set_layout<HorizontalBoxLayout>().set_spacing(10);

    auto sidebar = main_container->add_widget<Container>();
    sidebar->set_size({ { 20.0, Length::Percent }, Length::Auto });

    auto list = main_container->add_widget<ListView>();
    m_model = &list->create_and_set_model<FileModel>();

    list->on_click = [&](unsigned row) {
        open_path(m_model->get_path(row));
    };

    m_path_textbox->on_enter = [&](std::string path) {
        open_path(path);
    };

    parent_directory_button->on_click = [&]() {
        m_current_path = m_current_path.parent_path();
        m_path_textbox->set_content(m_current_path.string(), NotifyUser::No);
        m_model->update_content(m_current_path);
    };

    open_path("../worlds");
}

void FileExplorer::open_path(std::filesystem::path path) {
    if (!std::filesystem::is_directory(path)) {
        // TODO: Implement that
        std::cout << "select path: " << path << std::endl;
        return;
    }
    path = std::filesystem::absolute(path).lexically_normal();
    try {
        m_model->update_content(path);
    } catch (std::filesystem::filesystem_error& error) {
        m_model->update_content(m_current_path);
        GUI::message_box(error.path1().string() + ": " + error.code().message(), "Error!", GUI::MessageBox::Buttons::Ok);
        return;
    }
    m_current_path = path;
    m_path_textbox->set_content(path.string(), NotifyUser::No);
}

}
