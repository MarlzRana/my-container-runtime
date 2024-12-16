#include "container_minifs.hpp"
#include "constants.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <sys/wait.h>
#include <format>

// constexpr std::string_view ALPINE_LINUX_MINIFS_URL{"https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/aarch64/alpine-minirootfs-3.20.3-aarch64.tar.gz"};
// const std::filesystem::path CONTAINER_OVERLAY_FS{"/tmp/my-container-runtime"};
// const std::filesystem::path CONTAINER_ROOT{"/tmp/my-container-runtime/merged"};

void makeContainerOverlayFSDirectories() {
    // Make the directories that will make up the overlay file system
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS / "base");
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS / "diff");
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS / "merged");
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS / "scratchpad");
}

void curlMiniFileSystem() {
    // Let's fork so that we can execvp with output streams redirected to /dev/null
    pid_t pid = fork();

    // Child Process
    if (pid == 0) {
        // Redirect the child output streams to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Execute the curl
        char cmd[]{"curl"};
        char* const args[]{cmd, const_cast<char*>("-O"), const_cast<char*>(ALPINE_LINUX_MINIFS_URL.data()), const_cast<char*>("--output-dir"), const_cast<char*>(CONTAINER_OVERLAY_FS.c_str()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error("Error: There was an error whilst trying to curl the Alpine Linux image. {f:curlMiniFileSystem}");
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:curlMiniFileSystem)");
    }
}

void untarMiniFileSystem() {
    // Let's fork so that we can execvp with output streams redirected to /dev/null
    pid_t pid = fork();

    // Child Process
    if (pid == 0) {
        // Redirect the child output streams to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Execute the tar
        char cmd[]{"tar"};
        const std::filesystem::path tarFilePth{CONTAINER_OVERLAY_FS / std::string(ALPINE_LINUX_MINIFS_URL.substr(ALPINE_LINUX_MINIFS_URL.find_last_of('/') + 1))};
        const std::filesystem::path basePth{CONTAINER_OVERLAY_FS / "base"};
        char* const args[]{cmd, const_cast<char*>("xzf"), const_cast<char*>(tarFilePth.c_str()), const_cast<char*>("-C"), const_cast<char*>(basePth.c_str()), nullptr};
        std::cout << "Command: " << cmd << std::endl;
        std::cout << "Arguments: ";
        for (char* const* arg = args; *arg != nullptr; ++arg) {
            std::cout << *arg << " ";
        }
        std::cout << std::endl;
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error("Error: There was an error extracting the Alpine Linux image. (f:untarMiniFileSystem)");
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:untarMiniFileSystem)");
    }
}

void removeResidueTar() {
    // Let's fork so that we can execvp with output streams redirected to /dev/null
    pid_t pid = fork();


    // Child Process
    if (pid == 0) {
        // Redirect the child output streams to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Execute the rm
        char cmd[]{"rm"};
        const std::filesystem::path tarFilePth{CONTAINER_OVERLAY_FS / std::string(ALPINE_LINUX_MINIFS_URL.substr(ALPINE_LINUX_MINIFS_URL.find_last_of('/') + 1))};
        char* const args[]{cmd, const_cast<char*>(tarFilePth.c_str()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error("Error: There was an error removing the residue tar file. (f:removeResidueTar)");
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:removeResidueTar)");
    }
    
}

void createMiniFileSystem() {
    makeContainerOverlayFSDirectories();
    curlMiniFileSystem();
    untarMiniFileSystem();
    removeResidueTar(); 
}


void destroyMiniFileSystem() {
    // Let's fork so that we can execvp with output streams redirected to /dev/null
    pid_t pid = fork();

    // Child Process
    if (pid == 0) {
        // Redirect the child output streams to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Execute the mkdir
        char cmd[]{"rm"};
        char* const args[]{cmd, const_cast<char*>("-rf"), const_cast<char*>(CONTAINER_OVERLAY_FS.c_str()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error(std::format("Error: There was an error destroying the container root. Perhaps the directory {} has already been deleted.  (f:destroyMiniFileSystem)", CONTAINER_ROOT.c_str()));
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:destroyMiniFileSystem)");
    }
}