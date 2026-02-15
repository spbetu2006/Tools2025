#pragma once

#include <fstream>

#include "StreamProcessor.h"

// 0: Процессор по умолчанию просто выводит
class DefaultProcessor : public StreamProcessor {
public:
    DefaultProcessor(Stream &stream, size_t lines);
    virtual void process(std::ofstream &out) override;
protected:
    virtual void processLine(std::ofstream &out, size_t line) override;
    void processLineDefault(std::ofstream &out);
    void skipLineDefault();
private:
    size_t m_lines;
    bool   m_hasTab;
};
