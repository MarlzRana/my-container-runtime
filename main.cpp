#include "container_minifs.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <sys/wait.h>
#include <format>




int main() {
    createMiniFileSystem();
    destroyMiniFileSystem();

    return EXIT_SUCCESS;
}