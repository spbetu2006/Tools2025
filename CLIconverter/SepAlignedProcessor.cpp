#include "SepAlignedProcessor.h"

SepAlignedProcessor::SepAlignedProcessor(Stream &stream
                       , const tSepArray &sep
                       , size_t width) : DefaultProcessor(stream, sep[2]+1), m_width(width), m_seplines(sep) {
}

void SepAlignedProcessor::processLine(std::ofstream &out, size_t line) {
    if (!processLineSeparator(out, line)) 
        processLineDefault(out);
}

bool SepAlignedProcessor::processLineSeparator(std::ofstream &out, size_t cur_line) {
    const bool isSep = (cur_line == m_seplines);
    if (!isSep) {
        return false;
    }
    char ch, sep_ch = '=';
    m_stream.getChar(sep_ch);
    printChNtimes(out, sep_ch, m_width);
    while (m_stream.getChar(ch)) {
        if (ch == sep_ch) {
            continue;
        }
        if (isline(ch)) {
            out << std::endl;
            break;
        } else {
            out << ch;
        }
    }
    return true;
} 
