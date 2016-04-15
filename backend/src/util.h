#pragma once
#include <string>
#include <vector>
#include <algorithm>

#include "macros.h"

namespace util {
    std::vector<std::string> split_by_space(std::string in);

    std::string hexdecode(std::string in);

    std::string hexencode(std::string in);
}
