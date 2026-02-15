#pragma once

#include <fstream>

#include "common.h"
#include "Stream.h"
#include "tools.h"
#include "StreamProcessor.h"
#include "DefaultProcessor.h"

// 1: Процессор выравнивания сепараторов
class SepAlignedProcessor : public DefaultProcessor {
public:
    SepAlignedProcessor(Stream &stream
                       , const tSepArray &sep
                       , size_t width);
protected:
    virtual void processLine(std::ofstream &out, size_t line) override;
    bool processLineSeparator(std::ofstream &out, size_t cur_line);

    tSepArray m_seplines; // номера линий сепараторов
    size_t m_width;
private:
};
