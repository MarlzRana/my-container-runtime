#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string_view>
#include <filesystem>

extern const std::string_view ALPINE_LINUX_MINIFS_URL;
extern const std::filesystem::path CONTAINER_OVERLAY_FS;
extern const std::filesystem::path CONTAINER_OVERLAY_FS_BASE;
extern const std::filesystem::path CONTAINER_OVERLAY_FS_DIFF;
extern const std::filesystem::path CONTAINER_OVERLAY_FS_MERGED;
extern const std::filesystem::path CONTAINER_OVERLAY_FS_WORK;
extern const std::filesystem::path CONTAINER_ROOT;
extern const std::filesystem::path CONTAINER_CGROUP_PTH;


#endif