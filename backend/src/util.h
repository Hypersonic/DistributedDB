#pragma once
#include <string>
#include <vector>
#include <algorithm>

#include "macros.h"

namespace util {
    std::vector<std::string> split(std::string in, char splitchar);

    std::string join(std::vector<std::string> in, char joinchar);

    std::string hexdecode(std::string in);

    std::string hexencode(std::string in);

    bool contains(std::string s, std::string val);
}
