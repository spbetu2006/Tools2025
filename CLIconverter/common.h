#pragma once

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

extern const char * g_msgTabWarn;
extern const char * g_msgSepWarn;

typedef std::array<size_t,3> tSepArray;

namespace std {

vector<bool>& operator += (vector<bool> &one, const vector<bool>& two);
bool operator == (const size_t &one, const tSepArray  &two);

}

bool issep(char ch);
bool iscarriage(char ch);
bool isline(char ch);
bool istab(char ch);
bool isws(char ch);
bool isval(char ch);
bool isio(char ch);
