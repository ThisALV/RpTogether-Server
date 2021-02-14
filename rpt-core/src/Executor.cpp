#include <RpT-Core/Executor.hpp>

#include <iostream>

namespace RpT::Core {

Executor::Executor(std::vector<std::filesystem::path> game_resources_path, std::string game_name) {
    std::cout << "Game name : " << game_name << std::endl;

    for (const std::filesystem::path& resource_path : game_resources_path)
        std::cout << "Game resources path : " << resource_path << std::endl;
}

bool Executor::run() {
    std::cout << "Start." << std::endl;

    return true;
}

}