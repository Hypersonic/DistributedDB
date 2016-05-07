#include "util.h"

std::vector<std::string> util::split(std::string in, char splitchar) {
    std::vector<std::string> vec;
    std::string curr = "";
    for_each(in.begin(), in.end(), [&] (char c) {
            if (c == splitchar) {
                vec.push_back(curr);
                curr = "";
            } else {
                curr += c;
            }
            });
    vec.push_back(curr);
    return vec;
}

std::string util::join(std::vector<std::string> in, char joinchar) {
    std::string ret = "";
    for_each(in.begin(), in.end(), [&] (std::string s) {
        ret += s + joinchar;
    });
    return ret;
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

bool util::contains(std::string s, std::string val) {
    return s.find(val) != std::string::npos;
}
