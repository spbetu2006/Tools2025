#include <regex>
#include <iostream>
#include <cassert>
#include <optional>
#include <charconv>

using namespace std;

const string seq = "(1, 10) (2, 11), (3, 11), (4, 11), (5, 11) (6, 10) (7, 11) (8, 11) (9, 11) (10, 11) (11, 10)";
const size_t n = 4;

const regex rx("\\([ ]*([+-]?[\\d]+)[ ]*,[ ]*([+-]?[\\d]+)[ ]*\\)");

typedef pair<long long, long long> tPair;

void coutPair(long long x, long long y) {
    cout << " (" << x << ", " << y << ")";
}

int main(int argc, char* argv[]) {

    using It = sregex_iterator;
    optional<long long> prev_x;
    long long prev_y;

    assert(n > 2);
    size_t subseq_modlen = 0;
    bool outPrevPair = false;

    cout << "Исходная: " << seq << endl;
    cout << "Результат при n = " << n << ": ";

    for (It it(seq.begin(), seq.cend(), rx); it != It(); ++it) {
        auto m = *it;

        long long x, y;
        const auto& s1 = m[1].str();
        const auto& s2 = m[2].str();

        from_chars(s1.data(), s1.data() + s1.size(), x, 10);
        from_chars(s2.data(), s2.data() + s2.size(), y, 10);

        bool outCurPair = false;

        if (prev_x.has_value()) {
            assert(prev_x <= x);
 
            if (prev_y == y) {
                ++subseq_modlen;
                if ((n-1) == subseq_modlen) {
                    outCurPair = true;
                    subseq_modlen = 0; 
                }     
            } else {
                subseq_modlen = 0;
                outCurPair = true;
                if (!outPrevPair) {
                    coutPair(prev_x.value(), prev_y);
                }
            }
        } else {
           outCurPair = true;
        }
 
        if (outCurPair) {
            coutPair(x, y);
        }

        prev_x = x;
        prev_y = y;
        outPrevPair = outCurPair;
    }
    if (!outPrevPair) {
        coutPair(prev_x.value(), prev_y);
    }

    cout << endl;

    return 0;
}
