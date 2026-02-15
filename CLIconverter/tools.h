#pragma once

#include <fstream>
#include <vector>

#include "Stream.h"

// 2do: разнести отдельно на две "state_machine"
bool parseColumnsAndSeparator(Stream &stream            // Входной поток
                             , std::vector<size_t> &width    // Ширина колонок/столбцов	
                             , std::vector<bool>   &mask     // Маска символов
                             , std::vector<bool>   &seps     // Маска разделителей колонок/столбцов
                             , bool   &separator        // Флаг строки-разделителя
                             , char   &sep_ch);

void processDefault(Stream &stream, std::ofstream &out, size_t lines);

bool parseLineSeparator(Stream &stream, char &firstChar);

/*
** конвертация битовой маски в массив длин колонок
*/
void convertMask2Columns(const std::vector<bool> &mask, const std::vector<bool> &seps, std::vector<size_t> &width);

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
                   , char &sep_ch);           // Символ "строки-разделителя"

void printChNtimes(std::ofstream &out, char ch, size_t width);
