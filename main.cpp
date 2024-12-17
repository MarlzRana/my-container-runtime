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
    // Run everything in a new child process
    pid_t pid = fork();

    if (pid == 0) {
        // Child process

        // Turn off mount namespace propagation
        // MS_PRIVATE and MS_REC make sure that any changes to the mount point from sub-mounts are not propagated back to the parent mount (/) and vice versa
        if (mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL) == -1) {
            throw std::runtime_error("Error: Unable to turn off mount namespace propagation. (f:isolateAndRun)");
        }
        
        // Give this child process new namespaces (that are different from the parent)
        // CLONE_NEWPID gives the new child process a new PID namespace (with it having a pid of 1)
        // CLONE_NEWNS gives the new child process a copy of the parents mount namespace by value
        // So making changes in the childs mount namespace won't make changes to the parents mount namespace
        // However mount namespace change propagation may still be on between the parent and child
        // Hence we explicitly turn it off just before this
        // CLONE_NEWUTS gives the child process a copy of the Hostname and NIS domain name
        // CLONE_NEWIPC gives the child process a copy of the IPC namespace
        // There are things needed for IPC here like mutexes, queues, and semaphores
        // CLONE_NEWNET gives the child process its own network stack
        int namespacesToUnshare{CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWNET};
        if (unshare(namespacesToUnshare) == -1) {
            throw std::runtime_error("Error: Unable to unshare namespaces from parent to create container. (f:isolateAndRun)");
        }

        // Change the root
        if (chroot(const_cast<char*>(CONTAINER_ROOT.c_str())) != 0) {
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
        
        exit(EXIT_SUCCESS);        
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
    destroyMiniFileSystem();

    return EXIT_SUCCESS;
}