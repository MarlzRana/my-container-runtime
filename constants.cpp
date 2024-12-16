#include "constants.hpp"


constexpr std::string_view ALPINE_LINUX_MINIFS_URL{"https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/aarch64/alpine-minirootfs-3.20.3-aarch64.tar.gz"};
const std::filesystem::path CONTAINER_OVERLAY_FS{"/tmp/my-container-runtime"};
const std::filesystem::path CONTAINER_ROOT{"/tmp/my-container-runtime/merged"};