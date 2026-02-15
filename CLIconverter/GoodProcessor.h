#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <memory>

#include "common.h"
#include "Stream.h"
#include "tools.h"
#include "StreamProcessor.h"
#include "DefaultProcessor.h"
#include "SepAlignedProcessor.h"
#include "NiceProcessor.h"

// 3: Процессор выравнивания по маске при равном количестве колонок (в заголовке и в данных) 
//    Используется препроцессор

class GoodProcessor : public NiceProcessor {
private:
    enum EAlignment {
         EAlignment_Width,
         EAlignment_Mask,
         EAlignment_Shift
    };

    typedef std::pair<EAlignment, std::vector<size_t>> tColumnAlignment;
    typedef std::vector< tColumnAlignment > tAlignment;

public:
    class GoodPreprocessor : public DefaultProcessor {
    public:
        GoodPreprocessor(Stream &stream
                         , const tSepArray &sep
                         , const std::vector<size_t> &columns 
                         , const std::vector<size_t> &columns2
                         , bool nice, bool nice2
                         , tAlignment &alignment
                         , tAlignment &alignment2
                         , size_t &width
                         , std::vector<size_t> &merge_columns);

        virtual void process(std::ofstream &out) override;
        virtual void processLine(std::ofstream &out, size_t line) override;
        size_t calculateMaxWidth(bool nice
                                , const std::vector<size_t> &columns
                                , const std::vector< std::vector<size_t> > &mask
                                , const std::vector<size_t> &mins
                                , const std::vector<std::pair<size_t,size_t>> &startend);
        void preprocessLineGood(const std::vector<size_t> &columns
                                , std::vector< std::vector<size_t> > &mask
                                , std::vector<size_t> &mins
                                , std::vector<std::pair<size_t,size_t>> &startend);

    private:
        tSepArray m_seplines; // номера линий сепараторов

        /* Выравнивание колонки: 
        ** Если nice == true => 
        **      то выравнивание по ширине, используя (columns),
        ** Если nice == false =>
        **      то выравнивание по маске
        **      Если mins == mask.size(), 
        **           то подколонки выравниваются по ширине, используя (mask),
        **           NB! Если количество подколонок во всех строках равно (не считая строк без подколонок), то
        **           этот случай так же выравнивается по маске
        **      Если mins != mask.size(),
        **           то подколонки просто сдвигаются (без выравнивания),
        **           startend.first
        **
        ** Ширина колонки:
        ** Если nice == true =>
        **      то ширина колонки берется из columns
        ** Если nice == false =>
        **      то при выравнивании по маске ширина колонки берется из SUM(mask[i]) + mask[i].size - 1
        **      а при сдвиге (без выравнивания) ширина колонки startend.second - startend.first + 1
        */
        const std::vector< size_t > m_columns, m_columns2;
        std::vector< std::vector<size_t> >  m_mask, m_mask2;
        std::vector<size_t>            m_mins,  m_mins2;  // минималное (!=0) количество подколонок
        std::vector<std::pair<size_t, size_t>>      m_startend, m_startend2; // максимальная ширина колонки (по маске, без выравнивания)
        const bool m_nice, m_nice2;

        /*
        ** Масштабирование: для равномерного распределения колонок по маске
        */
        void rescheduleAlignmentWidth(tColumnAlignment &alignment);
        void setAlignment(const std::vector<size_t> &columns
                         , const std::vector<std::vector<size_t>> &mask
                         , const std::vector<size_t> &mins
                         , const std::vector<std::pair<size_t, size_t>> &startend
                         , bool nice
                         , tAlignment &alignment);
        void printAlignment(const tAlignment &alignment);

        /* Структура "alignment":
        ** - поле "тип выравнивания" (Width/Mask/Shift)
        ** - ширина результата (сколько поле будет занимать в выходном файле)
        ** - опционально: ширина поля во входной потоке
        ** - специфичные для типа выравнивания данные (позиция сдвига/ширина подколонок)
        */ 
        void mergeAlignment(tAlignment &alignment, tAlignment &alignment2);
        tAlignment &m_alignment, &m_alignment2;
        size_t &m_width;
        std::vector<size_t> &m_merge_columns;
    };

    GoodProcessor(Stream &stream
                 , const tSepArray &sep
                 , const std::vector<size_t> &columns
                 , const std::vector<size_t> &columns2
                 , bool nice
                 , bool nice2);
    virtual EProcessor getType() const override;
    virtual std::shared_ptr<StreamProcessor> getPreprocessor() const override;
    virtual void processLine(std::ofstream &out, size_t line) override;
    void processLineGood(std::ofstream &out, const tAlignment& alignment);

private:
    std::shared_ptr<StreamProcessor> m_preProcessor;
    tAlignment m_alignment, m_alignment2;
};
