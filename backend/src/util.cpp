#include "util.h"

std::vector<std::string> util::split_by_space(std::string in) {
    std::vector<std::string> vec;
    std::string curr = "";
    for_each(in.begin(), in.end(), [&] (char c) {
            if (c == ' ') {
            vec.push_back(curr);
            curr = "";
            } else if (c != '\n'){
            curr += c;
            }
            });
    vec.push_back(curr);
    return vec;
}

std::string util::hexdecode(std::string in) {
    std::string res = "";
    for (size_t i = 0; i < in.length(); i+=2) {
        std::string hexstr = "";
        hexstr += in[i];
        hexstr += in[i+1];
        if (hexstr.length() == 2) {
            res += std::stoi(hexstr, nullptr, 16);
        } else {
            ERR("wtf, this isn't hex: %s\n", hexstr.c_str());
        }
    }
    return res;
}

std::string util::hexencode(std::string in) {
    std::string res = "";
    char buf[3];
    for_each(in.begin(), in.end(), [&] (char c) {
        sprintf(buf, "%02x", c);
        res += buf;
    });
    return res;
}
