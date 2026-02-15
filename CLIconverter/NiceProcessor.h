#pragma once

#include <fstream>
#include <vector>

#include "common.h"
#include "Stream.h"
#include "tools.h"
#include "StreamProcessor.h"
#include "DefaultProcessor.h"
#include "SepAlignedProcessor.h"

// 4: Процессор выравнивания по колонкам (для заголовка и для данных) 
class NiceProcessor : public SepAlignedProcessor {
public:
    NiceProcessor(Stream &stream
                 , const tSepArray &sep 
                 , const std::vector<size_t> &columns);
protected:
    virtual void processLine(std::ofstream &out, size_t line) override;
    void processLineNice(std::ofstream &out);

    std::vector<size_t> m_columns;
private:
};

