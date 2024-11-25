#include "constants.hpp"
#include "container_minifs.hpp"

#include <cstdlib>
#include <linux/sched.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sys/wait.h>
#include <sys/mount.h>

void isolateAndRun() {
    // REMEMBER: The child process is the container

    // Turn off mount namespace propagation
    // MS_PRIVATE makes sure that mount namespace changes in children processes won't propagate to their parent
    // MS_REC makes sure that MS_PRIVATE is applied recursively to all mount points under / (or root)
    if (mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL) == -1) {
        std::runtime_error("Error: Unable to turn of mount namespace propagation. (f:isolateAndRun)");
    }
    
    // Give any child process new namespaces (that are different from the parent)
    // CLONE_NEWPID gives the new child process a new PID namespace (with it having a pid of 1)
    // CLONE_NEWNS gives the new child process a copy of the parents mount namespace by value
    // So making changes in the childs mount namespace won't make changes to the parents mount namespace
    // However mount namespace change propagation may still be on between the parent and child
    // Hence we explicitly turn it off just before this
    // CLONE_NEWUTS gives the child process a copy of the Hostname and NIS domain name
    // CLONE_NEWIPC gives the child process a copy of the IPC namespace
    // There things needs for IPC in here like mutexs, queues and semaphores
    // CLONE_NEWNET gives the child process its own network stack
    int namespacesToUnshare{CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWNET};
    if (unshare(namespacesToUnshare) == -1) {
        std::runtime_error("Error: Unable to unshare namespaces from parent to create container. (f:isolateAndRun)");
    }

    // Fork after unsharing
    pid_t pid = fork();

    if (pid == 0) {
        // Child process

        // Change the root
        if (chroot(const_cast<char*>(CONTAINER_ROOT.data())) != 0) {
            throw std::runtime_error("Error: Unable to change the root directory. (f:isolateAndRun)");
        }

        // Change the working directory to the new root
        if (chdir("/") != 0) {
            throw std::runtime_error("Error: Unable to change working directory to new root. (f:isolateAndRun)");
        }

        // Mount the proc file system
        if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
            throw std::runtime_error("Error: Unable to remount the proc file system. (f:isolateAndRun)");
        }
    } else if (pid > 0) {
        // Parent process - wait on the child to complete
        int childStatus{};
        waitpid(pid, &childStatus, 0);

        if (childStatus != 0) {
            throw std::runtime_error("Error: The isolated child process threw an error. (f:isolateAndRun)");
        }
    } else {
        throw std::runtime_error("Error: There was an error whilst trying fork. (f:isolateAndRun)");
    }


}

int main() {
    createMiniFileSystem();
    isolateAndRun();
    // destroyMiniFileSystem();

    return EXIT_SUCCESS;
}