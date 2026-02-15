#pragma once

#include <fstream>
#include <memory>

#include "common.h"
#include "Stream.h"
#include "tools.h"

/*
** Обработчики таблицы:
**   однопроходный, 
**   с препроцессором
*/
enum EProcessor {
     EProcessor_SingleRun = 0,
     EProcessor_DoubleRun
};

class StreamProcessor {
public:
    StreamProcessor(Stream &stream) : m_stream(stream) {}
    virtual void process(std::ofstream &out) = 0;
    virtual ~StreamProcessor() {}
    virtual EProcessor getType() const { return EProcessor_SingleRun; }
    virtual std::shared_ptr<StreamProcessor> getPreprocessor() const  { return NULL; }
protected:
    virtual void processLine(std::ofstream &out, size_t line) = 0;
    Stream &m_stream;

    // Набор вспомогательных функций обработки потока
    void processTail(std::ofstream &out) {
        char ch;
        while (m_stream.getChar(ch)) {
            if (isline(ch)) {
                out << std::endl;
                break;
            } else
                if (!isws(ch)) {
                    out << ch;
            }
        }
    }

private:
};
