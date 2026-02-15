#include "NiceProcessor.h"

NiceProcessor::NiceProcessor(Stream &stream
                 , const tSepArray &sep 
                 , const std::vector<size_t> &columns) : SepAlignedProcessor(stream, sep
                                                    , accumulate(columns.begin()
                                                    , columns.end(), 0) + columns.size() - 1), m_columns(columns) {
}

void NiceProcessor::processLine(std::ofstream &out, size_t line) {
    if (!processLineSeparator(out, line)) 
    {
        processLineNice(out);
    }
}

void NiceProcessor::processLineNice(std::ofstream &out) {

    char ch=0;
    for (size_t i{}; i<m_columns.size(); ++i) {
        const bool   isLastColumn = (i == (m_columns.size() - 1));
        size_t len = m_columns[i];
        bool isVal = false;

        // Считываем до начала колонки (или до конца линии)
        while (m_stream.getChar(ch)) {
            if ((isVal = isval(ch)))
                break;
            if (iscarriage(ch)) {
                out << ch;
            }
            if (isline(ch)) {
                out << std::endl;
                break;
            }
        }

        if (isline(ch) || m_stream.isEof())
            break;

        // Если найдена колонка, то выводим ее
        while (isVal) {
            out << ch;
            --len;
            if (!m_stream.getChar(ch)) {
                break;
            }
            isVal = isval(ch);
            if (isline(ch)) {
                out << std::endl;
            }
        }

        if (isline(ch) || m_stream.isEof())
            break;

        // Символы выравнивания
        if (!isLastColumn) {
            printChNtimes(out, ' ', len+1); 
        }
    }

    if (!m_stream.isEof() && !isline(ch)) {
        processTail(out);
    }
}
