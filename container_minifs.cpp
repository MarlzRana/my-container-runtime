#include "container_minifs.hpp"

#include "constants.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <sys/wait.h>
#include <format>


void makeContainerRoot() {
    // Let's fork so that we can execvp with output streams redirected to /dev/null
    pid_t pid = fork();

    // Child Process
    if (pid == 0) {
        // Redirect the child output streams to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        // Execute the mkdir
        char cmd[]{"mkdir"};
        char* const args[]{cmd, const_cast<char*>(CONTAINER_ROOT.data()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error(std::format("Error: There was an error making the container root. Perhaps the directory '{}' already exists. (f:makeContainerRoot)", CONTAINER_ROOT));
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:makeContainerRoot)");
    }
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
        char* const args[]{cmd, const_cast<char*>("-O"), const_cast<char*>(ALPINE_LINUX_MINIFS_URL.data()), const_cast<char*>("--output-dir"), const_cast<char*>(CONTAINER_ROOT.data()), nullptr};
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

        // Execute the curl
        char cmd[]{"tar"};
        std::string tarFilePath = std::string(CONTAINER_ROOT) + "/" + std::string(ALPINE_LINUX_MINIFS_URL.substr(ALPINE_LINUX_MINIFS_URL.find_last_of('/') + 1));
        char* const args[]{cmd, const_cast<char*>("xvzf"), const_cast<char*>(tarFilePath.c_str()), const_cast<char*>("-C"), const_cast<char*>(CONTAINER_ROOT.data()), nullptr};
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
        std::string tarFilePath = std::string(CONTAINER_ROOT) + "/" + std::string(ALPINE_LINUX_MINIFS_URL.substr(ALPINE_LINUX_MINIFS_URL.find_last_of('/') + 1));
        char* const args[]{cmd, const_cast<char*>(tarFilePath.c_str()), nullptr};
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
    makeContainerRoot();
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
        char* const args[]{cmd, const_cast<char*>("-rf"), const_cast<char*>(CONTAINER_ROOT.data()), nullptr};
        execvp(cmd, args);
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error(std::format("Error: There was an error destroying the container root. Perhaps the directory {} has already been deleted.  (f:destroyMiniFileSystem)", CONTAINER_ROOT));
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:destroyMiniFileSystem)");
    }
}