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

#include "common.h"

class Stream {
public:
    explicit Stream(std::ifstream &stream) : m_istream(stream), m_line(0), m_pos(0), m_hasTab(false) {
        savePos();
    }

    size_t getLine() const {
        return m_line;
    }

    size_t getPos() const {
        return m_pos;
    }

    bool getChar(char &ch) { 
         while (1) {
            if (isEof()) {
                return false;
            }
            m_istream.get(ch);

            if (isline(ch)) {
                ++m_line; 
                m_pos = 0;
                return true;
            } else /* TO-DO: Доработать функционал с табуляцией */ 
            if (istab(ch)) {
                ++m_pos;
                ch = 32;
                m_hasTab = true;
                return true;
            } else {
                ++m_pos;
                return true;
            }
        }

        return true;
    }

    bool hasTab() const {
         return m_hasTab;
    }

    void resetTab() {
         m_hasTab = false;
    }

    void savePos() {
         m_saved_tell = m_istream.tellg();
         m_saved_pos = m_pos;
         m_saved_line = m_line;
    }

    void restorePos() {
        m_istream.seekg(m_saved_tell, std::ios::beg);
        m_pos = m_saved_pos;
        m_line = m_saved_line;
    }

    bool isEof() const {
        return !m_istream.good() || m_istream.eof() || m_istream.fail() || m_istream.bad();
    }

protected:
    std::ifstream &m_istream;
    size_t m_line, m_pos;

    /*
    ** Данные для операций savePos / restorePos
    */
    size_t m_saved_line, m_saved_pos;
    std::streampos m_saved_tell;
    bool   m_hasTab;

private:
};

