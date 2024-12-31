# 🚀 My Container Runtime

This project is a simple container runtime implementation using C++. It provides basic functionalities to create, isolate, and run commands in a containerized environment using:

- Overlay File Systems
- Control Groups
- Process Namespace Isolation
- Change Root

## 📋 Prerequisites

- A Linux Machine/Virtual Machine
- g++-14
- Valgrind (for memory checking)
- sudo privileges

## 🛠️ Build and Run

### 🧑‍💻 Development Build

To build and run the project in development mode:

```sh
make dev ARGS="<command>"
```

### 🧹 Valgrind Build

To build and run the project with Valgrind for memory leak checking:

```sh
make dev-valgrind ARGS="<command>"
```

## 🗂️ Project Structure

- `main.cpp`: Entry point of the application and handles CLI argument pre-processing.
- `constants.cpp` and `constants.hpp`: Contains constant values used in the project.
- `container_overlayfs.cpp` and `container_overlayfs.hpp`: Handles the creation and destructions of the container overlay file system.
- `containerization.cpp` and `containerization.hpp`: Manages container creation, isolation, and execution.

## 🚀 Usage

Provide the command you want to run inside the container as an argument to the `make` command. For example:

```sh
make dev ARGS="echo Hello, World!"
```

## 💡 Motivation

When I was at Cisco Meraki, there was an engineer who was sparked my interest in UNIX-based operating systems. We spent countless evenings across whiteboards delving into the unique history of UNIX and how the decisions then have had so much influence on the computer engineering world today.

He once sent me a comic some random day over Slack. It was a single page and had 7 commands to create a container. I was like "7 commands? Is it really that simple?" and the answers is yes from a fundamental perspective. Obviously things like Docker do a lot more, but on a fundamental level this is what a container is (plus a bit more). So here is my C++ implementation of it and I dedicate this project to him ;)

## 📚 Helpful Information Sources

A huge shout out the individuals below. I'm not too sure how I would of otherwise got it done without these guides.

- [Run your own container without Docker](https://medium.com/@alexander.murylev/run-your-own-container-without-docker-60c297faf010) by [Alexander Murylev](https://www.linkedin.com/in/murylev/)
- [https://michalpitr.substack.com/p/primer-on-linux-container-filesystems](https://michalpitr.substack.com/p/primer-on-linux-container-filesystems) by [Michal Pitr](https://www.linkedin.com/in/michal-pitr-a7156b127/)
- [https://michalpitr.substack.com/p/linux-container-from-scratch](https://michalpitr.substack.com/p/linux-container-from-scratch) by [Michal Pitr](https://www.linkedin.com/in/michal-pitr-a7156b127/)

You may also find these references helpful during your adventures 🤠

- [Open Container Initiative](https://opencontainers.org/)
- [runc](https://github.com/opencontainers/runc)
- [How Docker for Mac Actually Spins Up a Linux Virtual Machine](https://news.ycombinator.com/item?id=30707599)
