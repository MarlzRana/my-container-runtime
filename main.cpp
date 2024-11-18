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
    
    // Give any child process new namespaces (that are different from the parent)
    // CLONE_NEWPID gives the new child process a new PID namespace (with it having a pid of 1)
    // CLONE_NEWNS gives the new child process a copy of the parents mount namespace by value
    // So making changes in the childs mount namespace won't make changes to the parents mount namespace
    // CLONE_NEWUTS gives the child process a copy of the Hostname and NIS domain name
    // CLONE_NEWIPC gives the child process a copy of the IPC namespace
    // There things needs for IPC in here like mutexs, queues and semaphores
    // CLONE_NEWNET gives the child process its own network stack
    int namespacesToUnshare{CLONE_NEWPID || CLONE_NEWNS || CLONE_NEWUTS || CLONE_NEWIPC || CLONE_NEWNET};
    if (unshare(namespacesToUnshare) == -1) {
        std::runtime_error("Error: Unable to unshare namespaces from parent to create container. (f:isolateAndRun)");
    }
}

int main() {
    createMiniFileSystem();
    isolateAndRun();
    destroyMiniFileSystem();

    return EXIT_SUCCESS;
}