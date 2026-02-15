#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <memory>

#include "GoodProcessor.h"

// 3: Процессор выравнивания по маске при равном количестве колонок (в заголовке и в данных) 
//    Используется препроцессор
GoodProcessor::GoodPreprocessor::GoodPreprocessor(Stream &stream
                         , const tSepArray &sep
                         , const std::vector<size_t> &columns 
                         , const std::vector<size_t> &columns2
                         , bool nice, bool nice2
                         , GoodProcessor::tAlignment &alignment
                         , GoodProcessor::tAlignment &alignment2
                         , size_t &width
                         , std::vector<size_t> &merge_columns) : DefaultProcessor(stream, sep[2]+1)
                                                          , m_columns(columns)
                                                          , m_columns2(columns2)
                                                          , m_nice(nice), m_nice2(nice2)
                                                          , m_seplines(sep)
                                                          , m_alignment(alignment), m_alignment2(alignment2), m_width(width), m_merge_columns(merge_columns) {
    if (!nice) {
        m_mask.resize(columns.size());
        m_mins.resize(columns.size());
        m_startend.resize(columns.size());
    }
    if (!nice2) {
        m_mask2.resize(columns2.size());
        m_mins2.resize(columns2.size());
        m_startend2.resize(columns2.size());
    }
}

void GoodProcessor::GoodPreprocessor::process(std::ofstream &out) {

    DefaultProcessor::process(out);

    // Устанавливаем выравнивание для заголовка и для данных
    setAlignment(m_columns, m_mask, m_mins, m_startend, m_nice, m_alignment);
    setAlignment(m_columns2,m_mask2,m_mins2,m_startend2,m_nice2,m_alignment2);

    // Приводим в соответствие выравнивание заголовка и данных
    mergeAlignment(m_alignment, m_alignment2);
}
        
void GoodProcessor::GoodPreprocessor::processLine(std::ofstream &out, size_t line) {
    const bool isSep = (line == m_seplines);
    if (isSep) {
        skipLineDefault();
    } else 
    if (!m_nice && line < m_seplines[1]) {
        preprocessLineGood(m_columns, m_mask, m_mins, m_startend);
    } else
    if (!m_nice2 && line > m_seplines[1]) {
        preprocessLineGood(m_columns2, m_mask2, m_mins2, m_startend2);
    } else {
        skipLineDefault();
    }
 }

size_t GoodProcessor::GoodPreprocessor::calculateMaxWidth(bool nice
                                , const std::vector<size_t> &columns
                                , const std::vector< std::vector<size_t> > &mask
                                , const std::vector<size_t> &mins
                                , const std::vector<std::pair<size_t,size_t>> &startend)
{
    if (nice)
        return std::accumulate(columns.begin(), columns.end(), 0) + columns.size() - 1;
    size_t max_width = 0;
    for (size_t i{}; i<mask.size(); ++i) {
         if (mask[i].size() == mins[i]) {
             max_width += std::accumulate(mask[i].begin(), mask[i].end(), 0) + mask[i].size() - 1;                    
         }  else {
             max_width += startend[i].second - startend[i].first + 1;
         }
    }
    max_width += mask.size() - 1;
    return max_width;
}

void GoodProcessor::GoodPreprocessor::preprocessLineGood(const std::vector<size_t> &columns
                                , std::vector< std::vector<size_t> > &mask
                                , std::vector<size_t> &mins
                                , std::vector<std::pair<size_t,size_t>> &startend) {

    // Препроцессинг:
    // - мы считаем кол-во слов в "стобце-маске" (оно должно быть равным для всях строк)
    // - мы считаем для каждого столбца ("явного" и "неявного") максимальную длину значения/слова 

    size_t cur_column = 0;
    size_t cur_bound =  columns[0], prev_bound = 0;
    char ch;
    size_t cur_internal_col = 0;
    size_t cur_internal_size = 0;
    bool newInternalColumn = true;  

    // Считываем колонки: сквозное чтение
    while (1) {
        m_stream.getChar(ch);
        const bool isBrkColumn = m_stream.isEof() || m_stream.getPos() > cur_bound || isline(ch);
                  
        if (!newInternalColumn) 
        {
            if (isBrkColumn || !isval(ch)) {
                // Обновляем максимум по длине и кол-ву "подколонок"
                if (cur_internal_col > mask[cur_column].size()) {
                    mask[cur_column].resize(cur_internal_col);
                }

                // 2do: оптимизация по ссылке
                if (cur_internal_size > mask[cur_column][cur_internal_col-1]) {
                    mask[cur_column][cur_internal_col-1] = cur_internal_size;
                }
            }
        }

        // Подсчет минимального ненулевого количества подколонок 
        // (если этот минимум совпадает с максимумом, то можно для подколонок использовать Nice/Good форматирование)
        if (isBrkColumn) {
            if ((!mins[cur_column] || cur_internal_col < mins[cur_column]) && cur_internal_col) {
                mins[cur_column] = cur_internal_col;
            }
        }

        if (m_stream.isEof())
            break;

        if (isline(ch)) {
            break;
        }

        // Сдвигаем границу (при необходимости)
        while (m_stream.getPos() > cur_bound) {
            ++cur_column;
            if (cur_column < columns.size()) {
                prev_bound = cur_bound;
                cur_bound += columns[cur_column];
                newInternalColumn = true;
                cur_internal_col = 0;
                cur_internal_size = 0;
            } else {
                skipLineDefault();
                return;
            }
        }

        if (isval(ch)) {
            const size_t cur_internal_pos = cur_column ? (m_stream.getPos() - prev_bound - 1) : m_stream.getPos();

            if (!startend[cur_column].first || cur_internal_pos < startend[cur_column].first) {
                startend[cur_column].first = cur_internal_pos;
            }
            if (!startend[cur_column].second || cur_internal_pos > startend[cur_column].second) {
                startend[cur_column].second = cur_internal_pos;
            }
            if (newInternalColumn) {
                ++cur_internal_col;
                newInternalColumn = false;
            }
            ++cur_internal_size;
        } else {
            newInternalColumn = true;
            cur_internal_size = 0;
        }
    }
}

/*
** Масштабирование: для равномерного распределения колонок по маске
*/
void GoodProcessor::GoodPreprocessor::rescheduleAlignmentWidth(tColumnAlignment &alignment) {

    if (EAlignment_Mask != alignment.first)
        return;

    if (alignment.second.size()<=2)
        return;

    // Считаем остаточную (не распределенную сумму) с учетом сепараторов
    const size_t sum_seps = alignment.second.size() - 2 - 1;
    const size_t sum_mask = std::accumulate(alignment.second.begin()+2, alignment.second.end(), 0);

    if (sum_mask == alignment.second[0] - sum_seps)
        return;

    // Распределение пропорционально размерности
    size_t rest_width = alignment.second[0] - sum_seps;
    for (size_t i=2; i<alignment.second.size(); ++i) {
        alignment.second[i] = ((alignment.second[0] - sum_seps) * alignment.second[i]) / sum_mask;
        rest_width -= alignment.second[i];
    }

    // Распределение остатка
    size_t i=2;
    while (rest_width) {
        ++alignment.second[i];
        --rest_width;
        ++i;
        if (i == alignment.second.size()) {
            i = 2;
        }
    }
}

void GoodProcessor::GoodPreprocessor::setAlignment(const std::vector<size_t> &columns
                         , const std::vector<std::vector<size_t>> &mask
                         , const std::vector<size_t> &mins
                         , const std::vector<std::pair<size_t, size_t>> &startend
                         , bool nice
                         , GoodProcessor::tAlignment &alignment) {

    alignment.resize(columns.size());
    if (nice == true) {
        for (size_t i{}; i<columns.size(); ++i) {
            alignment[i].first = EAlignment_Width;
            alignment[i].second.push_back(columns[i]);
        }
    } else {
        assert(columns.size() == mask.size());
        assert(columns.size() == mins.size());
        assert(columns.size() == startend.size());
        for (size_t i{}; i<columns.size(); ++i) {
            if (mins[i] == mask[i].size() || 0 == mins[i]) {
                const size_t output_len = mask[i].size() ? (std::accumulate(mask[i].begin(), mask[i].end(), 0) + mask[i].size()-1) : 0;
                alignment[i].first = EAlignment_Mask;
                alignment[i].second.reserve(2 + mask[i].size());
                alignment[i].second.push_back(output_len);
                alignment[i].second.push_back(columns[i]);
                copy(mask[i].begin(), mask[i].end(), back_inserter(alignment[i].second));        
            } else {
                alignment[i].first = EAlignment_Shift;
                alignment[i].second.push_back(startend[i].second - startend[i].first + 1);
                alignment[i].second.push_back(columns[i]);
                alignment[i].second.push_back(startend[i].first);
            }
        }
    }    
}

void GoodProcessor::GoodPreprocessor::printAlignment(const GoodProcessor::tAlignment &alignment) {
    for (size_t i{}; i<alignment.size(); ++i) {
        std::cout << "Alignment:";
        if (alignment[i].first == EAlignment_Width) std::cout << "Width"; else
        if (alignment[i].first == EAlignment_Mask) std::cout << "Mask"; else
        std::cout << "Shift";
        std::cout << " ";
        std::cout << "Output:" << alignment[i].second[0] << " ";
        if (alignment[i].first != EAlignment_Width)
        std::cout << "Input:" << alignment[i].second[1] << " ";
        if (alignment[i].first == EAlignment_Shift)
        std::cout << "Start:" << alignment[i].second[2] << " "; else
        if (alignment[i].first == EAlignment_Mask) {
            std::cout << "Subcolumns:";
            for (size_t j=2; j<alignment[i].second.size(); ++j) {
                 std::cout << alignment[i].second[j] << " ";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/* Структура "alignment":
** - поле "тип выравнивания" (Width/Mask/Shift)
** - ширина результата (сколько поле будет занимать в выходном файле)
** - опционально: ширина поля во входной потоке
** - специфичные для типа выравнивания данные (позиция сдвига/ширина подколонок)
*/ 
void GoodProcessor::GoodPreprocessor::mergeAlignment(GoodProcessor::tAlignment &alignment, GoodProcessor::tAlignment &alignment2) {

    assert(alignment.size() == alignment2.size());
    m_merge_columns.resize(alignment.size());
    m_width = 0;

    for (size_t i{}; i<alignment.size(); ++i) {
        assert(alignment[i].second.size());
        const size_t new_output = std::max(alignment[i].second[0], alignment2[i].second[0]);

        if (new_output > alignment[i].second[0]) {
            alignment[i].second[0] = new_output;
            // Перераспределение: после слияния длины масок для колонок могут увеличиться,
            // поэтому "вновь выделенное" место должно быть распределено между подколонками
        }
        rescheduleAlignmentWidth(alignment[i]);

        if (new_output > alignment2[i].second[0]) {
            alignment2[i].second[0] = new_output;
        }
        rescheduleAlignmentWidth(alignment2[i]);
 
        m_merge_columns[i] = alignment[i].second[0];
        m_width += m_merge_columns[i];
    }
    m_width += alignment.size()-1; 
}

GoodProcessor::GoodProcessor(Stream &stream
                 , const tSepArray &sep
                 , const std::vector<size_t> &columns
                 , const std::vector<size_t> &columns2
                 , bool nice
                 , bool nice2) : NiceProcessor(stream, sep, columns/*вычислим позже*/) {

    m_preProcessor = std::make_shared<GoodPreprocessor>(stream, sep, columns, columns2, nice, nice2, m_alignment, m_alignment2, m_width, m_columns); 
}

EProcessor GoodProcessor::getType() const { return EProcessor_DoubleRun; }

std::shared_ptr<StreamProcessor> GoodProcessor::getPreprocessor() const { return m_preProcessor; }

void GoodProcessor::processLine(std::ofstream &out, size_t line) {
    if (!processLineSeparator(out, line)) {
        const GoodProcessor::tAlignment &alignment = (line < m_seplines[1]) ? m_alignment : m_alignment2;
        if (EAlignment_Width == alignment[0].first) {
            processLineNice(out);
        } else {
            processLineGood(out, alignment);
        }
    }
}

void GoodProcessor::processLineGood(std::ofstream &out, const GoodProcessor::tAlignment& alignment) {
    size_t cur_column = 0;
    size_t cur_bound =  alignment[cur_column].second[1], prev_bound = 0;
    char ch;
    size_t cur_internal_col = 0;
    size_t cur_internal_size = 0;
    size_t cur_output_len = 0;
    bool newInternalColumn = true;

    // Считываем колонки: сквозное чтение
    while (1) {
        m_stream.getChar(ch);
        if (m_stream.isEof())
            break;
 
        // При форматировании "сдвиг" просто выводим все символы в заданном диапазоне из входного потока в выходной
        if (EAlignment_Shift == alignment[cur_column].first) {
            const size_t cur_internal_pos = m_stream.getPos() - prev_bound - 1;
            if (cur_internal_pos >= alignment[cur_column].second[2] 
                && cur_internal_pos < alignment[cur_column].second[2] + alignment[cur_column].second[0]) {

                out << ch;
            } 
        }

        if (isline(ch)) {
            out << std::endl;
            break;
        }

        // Во входном потоке пересекаем границу колонки
        while (m_stream.getPos() > cur_bound) {
            if (cur_column < alignment.size()) {
                if (EAlignment_Mask == alignment[cur_column].first && alignment[cur_column].second.size()) {
                    // дозаполнение строки подколонки для выравнивания
                    while (cur_output_len < alignment[cur_column].second[0]) {
                        ++cur_output_len;
                        out << ' ';
                    }
                }
            }

            ++cur_column;
            if (cur_column < alignment.size()) {
                prev_bound = cur_bound;
                cur_bound += alignment[cur_column].second[1];
                newInternalColumn = true;
                cur_internal_col = 0;
                cur_internal_size = 0;
                cur_output_len = 0;
                // вывод разделителя колонок
                out << ' ';
            } else {
                skipLineDefault();
                out << std::endl;
                return;
            }
        }

        if (isval(ch)) {
            if (newInternalColumn) {
                // вывод разделителя подколонок (при форматировании по маске)
                if (EAlignment_Mask == alignment[cur_column].first && cur_internal_col) {
                    out << ' ';
                    ++cur_output_len;
                }
                ++cur_internal_col;
                newInternalColumn = false;
            }
            ++cur_internal_size;

            if (EAlignment_Mask == alignment[cur_column].first) {
                out << ch;
                ++cur_output_len;
            }
        } else {
            if (EAlignment_Mask == alignment[cur_column].first && !newInternalColumn) {
                while (cur_internal_size < alignment[cur_column].second[2+cur_internal_col-1]) {
                    ++cur_internal_size;
                    out << ' ';
                    ++cur_output_len;
                }
            }
            newInternalColumn = true;
            cur_internal_size = 0;
        }
    }
}
