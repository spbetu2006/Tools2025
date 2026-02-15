#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <tuple>
#include <vector>
#include <memory>
#include <clocale>

#include "NiceProcessor.h"
#include "GoodProcessor.h"

using namespace std;

shared_ptr<StreamProcessor> parseTable(Stream &stream, size_t &lines_skip) {

    char sep_ch;
    if (!parseLineSeparator(stream, sep_ch)) {
        ++lines_skip;
        return NULL;
    }

    size_t lines = 0;
    vector<size_t> width;
    size_t max_columns = 0;
    size_t max_width = 0;
    bool   is_nice = false;
    vector<bool> mask, seps;
    char sep_ch2;
    if (!parseSubTable(stream, width, lines, max_columns, max_width, is_nice, mask, seps, sep_ch2)) {
        lines_skip += 1 + lines;
        return NULL;
    }
    if (sep_ch != sep_ch2) {
        cout << g_msgSepWarn << endl;
        lines_skip += lines;
        return NULL;
    }

    size_t lines2 = 0;
    vector<size_t> width2;
    size_t max_columns2 = 0;
    size_t max_width2 = 0;
    bool   is_nice2 = false;
    vector<bool> mask2, seps2;
    char sep_ch3;
    if (!parseSubTable(stream, width2, lines2, max_columns, max_width2, is_nice2, mask2, seps2, sep_ch3)) {
        lines_skip += 1 + lines + lines2;
        return NULL;
    }
    if (sep_ch3 != sep_ch2) {
        cout << g_msgSepWarn << endl;
        lines_skip += lines + lines2;
        return NULL;
    }

    cout << "Table recognized header: " << lines << " lines " << width.size() << " columns, data: " << lines2 << " lines " << width2.size() << " columns";

    // Равное количество колонок/столбцов в заголовке и данных
    if (width.size() == width2.size()) {
 
        if (is_nice && is_nice2) {
            for (size_t i{}; i<width.size(); ++i) {
                if (width2[i] > width[i]) {
                    width[i] = width2[i];
                }
            }

            cout << "   ... NICE formatting" << endl;
            return make_shared<NiceProcessor>(stream, tSepArray({0, lines, lines+lines2}), width);
        }

        cout << "   ... GOOD formatting" << endl; 
        return make_shared<GoodProcessor>(stream, tSepArray({0, lines, lines+lines2}), width, width2, is_nice, is_nice2);
    }

    mask += mask2;
    seps += seps2; 
    convertMask2Columns(mask, seps, width);
    width2 = width;
    cout << "    ... BAD formatting" << endl;
    return make_shared<GoodProcessor>(stream, tSepArray({0, lines, lines+lines2}), width, width2, false, false);
}

#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

/* 
** Парсинг процедурный:
** - lines_skip линий выводятся без процессинга (processDefault); 
** - lines_proc линий выводятся с процессингом (функция преобразования входного потока в выходной);
*/
tuple<function<shared_ptr<StreamProcessor> (Stream&, size_t&)>> g_parse[] =
{
   { parseTable }
};

void parseStream(Stream &stream, ofstream &out) {

   while (!stream.isEof()) {
       stream.savePos();
       for (size_t i{}; i<ARRAY_SIZE(g_parse); ++i) {
           size_t lines_skip = 0;
           shared_ptr<StreamProcessor> sp = get<0>(g_parse[i])(stream, lines_skip);
           stream.restorePos();
           //2do: optimize performance new/delete
           // Вариант-1: структура распознана, создан процессор для обработки входного потока
           if (sp) {
               // Вариант-1.1: обычный процессор (обработка потока в один проход)
               if (EProcessor_SingleRun == sp->getType()) {
                   sp->process(out);
               } else 
               // Вариант-1.2: обработка с препроцессором (в два прохода)
               if (EProcessor_DoubleRun == sp->getType())
               {
                   // первый проход
                   shared_ptr<StreamProcessor> sprep = sp->getPreprocessor();
                   assert(sprep);
                   sprep->process(out);
                
                   // второй проход
                   stream.restorePos();
                   sp->process(out);
               }
           } else 
           // Вариант-2: структура не распознана (следующая попытка распознать через lines_skip линий)
           {
               processDefault(stream, out, lines_skip);
           }
       }
   }
}

int main(int argc, char* argv[]) {

    if (argc<2) {
        cout << "Usage: " << argv[0] << " <table_file_input>" << endl;
    }

    // для ускорения считывания
    ios::sync_with_stdio(false);
    cin.tie(nullptr); cout.tie(nullptr);

    std::setlocale(LC_ALL, "en_US.UTF-8");
 
    std::ifstream in(argv[1]); 
    if (in.fail())
        return 1;

    in.imbue(locale("en_US.UTF-8"));

    string outFileName = argv[1];
    outFileName += ".out";
    std::ofstream out(outFileName);

    out.imbue(locale("en_US.UTF-8"));

    Stream str(in);
    parseStream(str, out);

    return 0;
}
