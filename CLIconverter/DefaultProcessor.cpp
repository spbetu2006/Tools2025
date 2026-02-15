#include <fstream>

#include "DefaultProcessor.h"

DefaultProcessor::DefaultProcessor(Stream &stream, size_t lines) : StreamProcessor(stream), m_lines(lines), m_hasTab(false) {}

void DefaultProcessor::process(std::ofstream &out) {
    m_stream.resetTab();
    for (size_t i{}; i<m_lines; ++i) {
        processLine(out, i);
    }
    if (m_stream.hasTab()) {
        std::cout << g_msgTabWarn << std::endl;
    }
    m_stream.resetTab();
}

void DefaultProcessor::processLine(std::ofstream &out, size_t line) {
    processLineDefault(out);
} 

void DefaultProcessor::processLineDefault(std::ofstream &out) {
    char ch;
    while (m_stream.getChar(ch)) {
       if (isline(ch)) {
           out << std::endl;
           break;
       } else {
           out << ch;
       }
    }
}

void DefaultProcessor::skipLineDefault() {
    char ch;
    while (m_stream.getChar(ch)) {
        if (isline(ch)) {
            break;
        }
    }
}
