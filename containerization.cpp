#include "containerization.hpp"
#include "constants.hpp"

#include <array>
#include <cstdlib>
#include <linux/sched.h>
#include <iostream>
#include <filesystem>
#include <format>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>

void setupControlGroup() {
    // It's very hard to delete a control group, so on multiple runs lets use the same control group
    if (!std::filesystem::exists(CONTAINER_CGROUP_PTH / "my-container-runtime")) {
        std::filesystem::create_directories(CONTAINER_CGROUP_PTH / "my-container-runtime");
    }

    // Enable the memory and CPU memory controllers in the control group
    std::ofstream cgroupControlFile(CONTAINER_CGROUP_PTH / "cgroup.subtree_control");
    if (!cgroupControlFile.is_open()) {
        throw std::runtime_error("Error: Unable to open cgroup.subtree_control file. (f:setupControlGroups)");
    }
    cgroupControlFile << "+memory +cpu";
    cgroupControlFile.close();

    // Use every 10000 of 100000 cpu time
    // 100000 represents the max available CPU seconds for a single CPU core
    // However in reality, on a multi-core system, these 10000 seconds could be split across multiple machines
    std::ofstream cpuMaxFile(CONTAINER_CGROUP_PTH / "my-container-runtime/cpu.max");
    if (!cpuMaxFile.is_open()) {
        throw std::runtime_error("Error: Unable to open cpu.max file. (f:setupControlGroups)");
    }
    cpuMaxFile << "10000 100000";
    cpuMaxFile.close();

    // Restricts the memory usage of the container to 500MB
    std::ofstream memoryMaxFile(CONTAINER_CGROUP_PTH / "my-container-runtime/memory.max");
    if (!memoryMaxFile.is_open()) {
        throw std::runtime_error("Error: Unable to open memory.max file. (f:setupControlGroups)");
    }
    memoryMaxFile << "500M";
    memoryMaxFile.close();

    // Disables memory swap
    std::ofstream memorySwapMaxFile(CONTAINER_CGROUP_PTH / "my-container-runtime/memory.swap.max");
    if (!memorySwapMaxFile.is_open()) {
        throw std::runtime_error("Error: Unable to open memory.swap.max");
    }
    memorySwapMaxFile << "0";
    memorySwapMaxFile.close();
}

void assignControlGroup() {
    // Puts the current process in the control group
    std::ofstream cgroupProcsFile(CONTAINER_CGROUP_PTH / "my-container-runtime/cgroup.procs");
    if (!cgroupProcsFile.is_open()) {
        throw std::runtime_error("Error: Unable to open cgroup.procs file. (f:assignControlGroup)");
    }
    cgroupProcsFile << getpid();
    cgroupProcsFile.close();
}

void unshareNamespaces() {
    // Turn off mount namespace propagation
    // MS_PRIVATE and MS_REC make sure that any changes to the mount point from sub-mounts are not propagated back to the parent mount (/) and vice versa
    if (mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL) != 0) throw std::runtime_error("Error: Unable to turn off mount namespace propagation. (f:isolateAndRun)");

    // Give a child process new namespaces (that are different from the parent)
    // CLONE_NEWPID gives the new child process a new PID namespace (with it having a pid of 1)
    // CLONE_NEWNS gives the new child process a copy of the parents mount namespace by value
    // So making changes in the childs mount namespace won't make changes to the parents mount namespace
    // However mount namespace change propagation may still be on between the parent and child
    // Hence we explicitly turn it off just before this
    // CLONE_NEWUTS gives the child process a copy of the Hostname and NIS domain name
    // CLONE_NEWIPC gives the child process a copy of the IPC namespace
    // There are things needed for IPC here like mutexes, queues, and semaphores
    // CLONE_NEWNET gives the child process its own network stack
    int namespacesToUnshare{CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWCGROUP};
    if (unshare(namespacesToUnshare) != 0) throw std::runtime_error("Error: Unable to unshare namespaces from parent to create container. (f:isolateAndRun)");
}

void changeRoot() {
    // Change the root
    if (chroot(const_cast<char*>(CONTAINER_ROOT.c_str())) != 0) throw std::runtime_error("Error: Unable to change the root directory. (f:isolateAndRun)");

    // Change the working directory to the new root
    if (chdir("/") != 0) throw std::runtime_error("Error: Unable to change working directory to new root. (f:isolateAndRun)");    
}

void makeSpecialDevices() {
    // Setup the default devices in /dev
    // It takes in a mode as its second parameter where the the first 4 numbers are options for mknod itself
    // And the last 4 numbers are to set the permissions of the special file we create for the device
    // It then takes in a dev_t which specifies the major and minor for the device
    // The major selects the device driver
    // The minor selects the kind of device
    // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/devices.txt
    // /dev/null is used as data void - anything that gets put in here disappears essentially
    // /dev/zero is an infinite string of null(zero) bytes
    // /dev/tty represents the controlling terminal for the current process and allows the current process to interact with the terminal
    // Works in a very similar way to std::cout/std::cin - I suspect this what is it using under the hood
    if (mknod("/dev/null", 0666 | S_IFCHR, ((static_cast<dev_t>(1) << 8)| 3)) != 0) throw std::runtime_error("Error: Unable to create /dev/null (f:isolateAndRun)");
    if (mknod("/dev/zero", 0666 | S_IFCHR, ((static_cast<dev_t>(1) << 8)| 5)) != 0) throw std::runtime_error("Error: Unable to create /dev/zero (f:isolateAndRun)");
    if (mknod("/dev/tty", 0666 | S_IFCHR, ((static_cast<dev_t>(5) << 8)| 0)) != 0) throw std::runtime_error("Error: Unable to create /dev/tty (f:isolateAndRun)");
}

void mountFileSystems() {
    // Make dev/shm and dev/pts
    std::filesystem::create_directories("/dev/shm");
    std::filesystem::create_directories("/dev/pts");

    // Mount the proc file system
    if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
        throw std::runtime_error("Error: Unable to remount the proc file system. (f:isolateAndRun)");
    }

    // Mount devpts file system
    // /dev/pts is used for pseudo-terminals, enabling things like terminal emulation and remote terminal access
    if (mount("devpts", "/dev/pts", "devpts", 0, NULL) != 0) {
        throw std::runtime_error("Error: Unable to mount devpts file system. (f:isolateAndRun))");
    }

    // Mount the tmpfs file system in a few directories
    // /dev/shm is a temporary file system typically used for IPC
    // /run is used for storing runtime data for both the OS and other applications
    std::array<const char *, 3> tmpFsDirs{{"/tmp", "/run", "/dev/shm"}};
    for (const char* dir: tmpFsDirs) {
        if (mount("tmpfs", dir, "tmpfs", 0, NULL) != 0) {
            throw std::runtime_error(std::format("Error: Unable to mount the tmpfs file system in {}. (f:isolateAndRun)", dir));
        }    
    }

    // Mount the sysfs file system
    // Exposes kernel objects, attributes and their relationships to userspace in /sys
    if (mount("sysfs", "sys/", "sysfs", 0, NULL) != 0) {
        throw std::runtime_error("Error: Unable to remount the sys file system. (f:isolateAndRun)");
    }
}

void container::isolateAndRun(std::string& command) {

    setupControlGroup();

    assignControlGroup();

    unshareNamespaces();

    // Run everything in a new child process
    pid_t pid = fork();

    if (pid == 0) {
        // Child process

        changeRoot();

        makeSpecialDevices();

        mountFileSystems();

        system(command.c_str());
        
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
