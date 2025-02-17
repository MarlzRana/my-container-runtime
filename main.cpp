#include "constants.hpp"
#include "container_overlayfs.hpp"
#include "containerization.hpp"

#include <iostream>
#include <string>


int main(int argc, char* argv[]) {
    #if DEBUG
    std::cout << std::unitbuf;
    #endif
    if (argc < 2) {
        throw std::runtime_error("Error: Please provide a command.");
    }

    // Ignore the first argument
    --argc;
    ++argv;

    std::string command{};
    command += argv[0];
    for(int i{1}; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }

    container::createFileSystem();
    container::isolateAndRun(command);
    container::destroyFileSystem();

    return EXIT_SUCCESS;
}