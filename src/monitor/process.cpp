//
// Created by Aymane on 7/26/2023.
//

#include "process.h"
#include <iostream>
#include <sys/stat.h>
#include <pwd.h>
#include <cerrno>
#include <cstring>


std::string Process::userName(const int pid) {
    const std::string path = "/proc/" + std::to_string(pid);
    struct stat sb{};

    if (stat(path.c_str(), &sb) == -1) {
        std::cerr << "Error calling stat on: " << path << " - " << std::strerror(errno) << std::endl;
        return "";
    }

    struct passwd *pw = getpwuid(sb.st_uid);
    if (pw && pw->pw_name) {
        return pw->pw_name;
    } else {
        std::cerr << "Error getting username from UID - " << std::strerror(errno) << std::endl;
        return "";
    }
}
