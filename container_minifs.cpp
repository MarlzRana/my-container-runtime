#include "container_minifs.hpp"
#include "constants.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <format>

void makeContainerOverlayFSDirectories() {
    // Make the directories that will make up the overlay file system
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS_BASE);
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS_DIFF);
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS_MERGED);
    std::filesystem::create_directories(CONTAINER_OVERLAY_FS_WORK);
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
        char* const args[]{cmd, const_cast<char*>("xzf"), const_cast<char*>(tarFilePth.c_str()), const_cast<char*>("-C"), const_cast<char*>(CONTAINER_OVERLAY_FS_BASE.c_str()), nullptr};
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

void mountOverlayFS() {
    // When creating an overlay filesystem mount the source is "overlay" and the target is the merged directory
    std::string mountOptions{std::format("lowerdir={},upperdir={},workdir={}", CONTAINER_OVERLAY_FS_BASE.c_str(), CONTAINER_OVERLAY_FS_DIFF.c_str(), CONTAINER_OVERLAY_FS_WORK.c_str())};
    if (mount("overlay", const_cast<char*>(CONTAINER_OVERLAY_FS_MERGED.c_str()), "overlay", 0, const_cast<char*>(mountOptions.c_str())) != 0) {
        throw std::runtime_error("Error: Could not mount overlay file system. (f:mountOverlayFS)");
    }
    
}

void createMiniFileSystem() {
    makeContainerOverlayFSDirectories();
    curlMiniFileSystem();
    untarMiniFileSystem();
    removeResidueTar();
    mountOverlayFS();
}


void destroyMiniFileSystem() {
    // Delete the default devices in dev
    pid_t pid = fork();
    if (pid == 0) {
        const std::string devNullPth{(CONTAINER_OVERLAY_FS_MERGED / "dev" / "null").c_str()};
        const std::string devZeroPth{(CONTAINER_OVERLAY_FS_MERGED / "dev" / "zero").c_str()};
        const std::string devTtyPth{(CONTAINER_OVERLAY_FS_MERGED / "dev" / "tty").c_str()};
        char* const cmdArgs[]{const_cast<char *>("rm"), const_cast<char*>(devNullPth.c_str()), const_cast<char*>(devZeroPth.c_str()), const_cast<char*>(devTtyPth.c_str()), nullptr};
        execvp(cmdArgs[0], cmdArgs);
    } else if (pid > 0) {
        int childStatus{};
        waitpid(pid, &childStatus, 0);
        if (childStatus != 0) {
            throw std::runtime_error("Error: There was an error deleting the default devices in the container. (f:destroyMiniFileSystem))");
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork when trying to delete the default devices. (f:destroyMiniFileSystem)");
    }


    // Let's unmount the overlay file system
    if(umount(const_cast<char*>(CONTAINER_OVERLAY_FS_MERGED.c_str())) != 0) {
        throw std::runtime_error("Error: Could not unmount overlay file system. (f:destroyMiniFileSystem)");
    }

    pid = fork();
    if (pid == 0) {
        char cmd[]{"rm"};
        char* const args[]{cmd, const_cast<char*>("-rf"), const_cast<char*>(CONTAINER_OVERLAY_FS.c_str()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        int childStatus{};
        waitpid(pid, &childStatus, 0);
        if (childStatus != 0) {
            throw std::runtime_error(std::format("Error: There was an error destroying the container root. Perhaps the directory {} has already been deleted.  (f:destroyMiniFileSystem)", CONTAINER_OVERLAY_FS.c_str()));
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork when trying to delete the container overlay filesystem. (f:destroyMiniFileSystem)");
    }
}