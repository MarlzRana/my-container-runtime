#include "constants.hpp"


constexpr std::string_view ALPINE_LINUX_MINIFS_URL{"https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/aarch64/alpine-minirootfs-3.20.3-aarch64.tar.gz"};
const std::filesystem::path CONTAINER_OVERLAY_FS{"/tmp/my-container-runtime"};
const std::filesystem::path CONTAINER_OVERLAY_FS_BASE{CONTAINER_OVERLAY_FS / "base"};
const std::filesystem::path CONTAINER_OVERLAY_FS_DIFF{CONTAINER_OVERLAY_FS / "diff"};
const std::filesystem::path CONTAINER_OVERLAY_FS_MERGED{CONTAINER_OVERLAY_FS / "merged"};
const std::filesystem::path CONTAINER_OVERLAY_FS_WORK{CONTAINER_OVERLAY_FS / ".work"};
const std::filesystem::path CONTAINER_ROOT{CONTAINER_OVERLAY_FS_MERGED};
const std::filesystem::path CONTAINER_CGROUP_PTH{"/sys/fs/cgroup/mycontainerruntime.slice/"};
