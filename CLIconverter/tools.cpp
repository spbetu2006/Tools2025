#include "tools.h"

// 2do: разнести отдельно на две "state_machine"
bool parseColumnsAndSeparator(Stream &stream            // Входной поток
                             , std::vector<size_t> &width    // Ширина колонок/столбцов	
                             , std::vector<bool>   &mask     // Маска символов
                             , std::vector<bool>   &seps     // Маска разделителей колонок/столбцов
                             , bool   &separator        // Флаг строки-разделителя
                             , char   &sep_ch) {        // Символ строки-разделителя

    size_t len = 0;
    bool newColumn = true;
    char ch;
    width.clear();
    separator = true;

    while (stream.getChar(ch)) {
        if (isline(ch)) {
            break;
        }
        if (isval(ch)) {
            separator &= issep(ch);
            if (separator) {
                sep_ch = ch;
            }
            if (stream.getPos() >= mask.size()) {
                mask.resize(stream.getPos()+1);
            } 
            if (stream.getPos() >= seps.size()) {
                seps.resize(stream.getPos()+1);
            }
            mask[stream.getPos()-1] = true;
            if (newColumn) {
                if (!width.size()) {
                    separator &= (1 == stream.getPos());
                }
                if (len) {
                    width.push_back(len);
                }
                newColumn = false;
                len=0;
            } else {
                seps[stream.getPos()-1] = true;
            }
            ++len;
        } else {
            newColumn = true;
        }
    }

    if (len) {
       width.push_back(len);
    }

    if (!width.size()) {
       separator = false;
    }

    return true;
}

void processDefault(Stream &stream, std::ofstream &out, size_t lines) {

    while (lines) {
        char ch;
        while (stream.getChar(ch)) {
            if (isline(ch)) {
                out << std::endl;
                break;
            } else {
                out << ch;
            }
        }
        --lines;
    }
}

bool parseLineSeparator(Stream &stream, char &firstChar) {

    if (stream.getChar(firstChar)) {
        if (!issep(firstChar)) {
            return false;
        }
        char ch;
        while (stream.getChar(ch)) {
            if (ch != firstChar) {
                break;
            }
        }
        do {
            if (isline(ch)) {
                return true;
            }
            if (!isspace(ch) && !iscarriage(ch)) {
                return false;
            }
            stream.getChar(ch);
        } while (1); 
        return true;
    }     
    return false;    
}

/*
** конвертация битовой маски в массив длин колонок
*/
void convertMask2Columns(const std::vector<bool> &mask, const std::vector<bool> &seps, std::vector<size_t> &width) {

     width.clear();
     bool newColumn = false;
     size_t columnSize = 0;
     for (size_t i{}; i<mask.size();++i) {
         if (mask[i]) {
             newColumn = true;
         }
         if (!mask[i]) {
             if (newColumn && !seps[i]) {
                 width.push_back(columnSize);
                 columnSize = 0;
                 newColumn = false;
             }
         }
         ++columnSize;
    }
    if (columnSize && newColumn) {
        width.push_back(columnSize);
    }
}

/* 
** статус bool - субтаблица распознана
** lines - кол-во линий без учета финального сепаратора
*/
bool parseSubTable(Stream &stream              // Входной поток
                   , std::vector<size_t> &width     // Максимальная ширина каждой колонки (по маске или подсчетом)
                   , size_t &lines             // Кол-во "обработанных" линий
                   , size_t &max_columns       // Максимально количество колонок (отличается от width.size() при работе по маске)
                   , size_t &max_width         // Положение крайнего (правого) символа в таблице
                   , bool   &isNiceFormat      // Флаг выравнивания по количеству колонок (без маски, без пропусков, совпадение по всем строкам)
                   , std::vector<bool> &mask        // Для выравнивания-сдвига: маска символов
                   , std::vector<bool> &seps        // Для выравнивания-сдвига: маска разделителей 
                   , char &sep_ch) {           // Символ "строки-разделителя"

    isNiceFormat = true;
    mask.clear();
    seps.clear();
    bool separator;
    max_columns = 0;
    max_width = 0;

    // Считываем первую линию с данными (считаем колонки)
    const bool res2 = parseColumnsAndSeparator(stream, width, mask, seps, separator, sep_ch);
    ++lines;
    if (!res2) {
        return false;
    }
    // 2do: двойное считывание сепаратора???
    if (separator) {
        return true;
    }
    if (width.size()>max_columns) {
        max_columns = width.size();
    }

    // Считываем последующие линии с данными (до линии-разделителя)
    while (!stream.isEof()) {
        std::vector<size_t> width2;
        std::vector<bool> mask2, seps2;
        const bool res3 = parseColumnsAndSeparator(stream, width2, mask2, seps2, separator, sep_ch);
        ++lines;
        if (!res3) {
            return false;
        }
        if (separator) {
            break;
        }
        // Обновление максимума
        if (width2.size()>max_columns) {
            max_columns = width2.size();
        }

        // Обновляем ширину столбцов по максимуму
        if (isNiceFormat) {
            if (width.size() == width2.size()) {
                for (size_t i{}; i<width2.size(); ++i) {
                    if (width2[i] > width[i]) {
                        width[i] = width2[i];
                    }
                }
            } else {
                isNiceFormat = false;
            }
        }

        // Обновляем маску столбцов
        mask += mask2;
        seps += seps2;
    }    

    max_width = mask.size();

    // При несовпадении числа колонок в строках считаем ширину колонок (width) по маске (mask)
    if (!isNiceFormat) {
       convertMask2Columns(mask, seps, width);
    } 

    return (width.size() > 0);
}

void printChNtimes(std::ofstream &out, char ch, size_t width) {
    for (size_t i{};i<width;++i) {
        out << ch;
    }
}
