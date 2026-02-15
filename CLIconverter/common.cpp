#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <vector>
#include <cctype>
#include <array>
#include <numeric>
#include <algorithm>
#include <memory>

#include "common.h"

const char * g_msgTabWarn = "Warning: TAB character discover. Might lead to unwanted misalignment";
const char * g_msgSepWarn = "Warning: different separator characters are used. Skipping table";

typedef std::array<size_t,3> tSepArray;

namespace std {

vector<bool>& operator += (vector<bool> &one, const vector<bool>& two) {
 
    if (two.size() > one.size()) {
        one.resize(two.size());
    }

    for (size_t i{}; i<two.size(); ++i) {
        one[i] = one[i] | two[i];
    }

    return one;
}

bool operator == (const size_t &one, const tSepArray  &two) {

    if (one == two[0] || one == two[1] || one == two[2])
        return true;
    return false;
} 

}

bool issep(char ch) {
    return '-' == ch || '=' == ch;
}

bool iscarriage(char ch) {
    return '\r' == ch;
}

bool isline(char ch) {
    return '\n' == ch;
}

bool istab(char ch) {
    return '\t' == ch;
}

bool isws(char ch) {
    return 32 == ch;
}

bool isval(char ch) {
    return !iscarriage(ch) && !isline(ch) && !istab(ch) && !isws(ch);
}

bool isio(char ch) {
    return isline(ch) || iscarriage(ch);
}
