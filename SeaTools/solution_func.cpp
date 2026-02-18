#include <regex>
#include <iostream>
#include <cassert>
#include <optional>
#include <charconv>
#include <vector>
#include <utility>
#include <iterator>

using namespace std;

const string seq = "(1, 10) (2, 11), (3, 11), (4, 11), (5, 11) (6, 10) (7, 11) (8, 11) (9, 11) (10, 11) (11, 10)";
const size_t n = 4;

const regex rx("\\([ ]*([+-]?[\\d]+)[ ]*,[ ]*([+-]?[\\d]+)[ ]*\\)");

typedef pair<long long, long long> tPair;
typedef vector<tPair> tSequence;

tSequence::iterator delBetweenAndPosition(tSequence &deq, tSequence::iterator &it_from, tSequence::iterator &it_to) {

    if (it_to == it_from)
        return it_to;

    tSequence::iterator next_it = it_from;
    advance(next_it, 1);

    if (next_it != it_to) {
        deq.erase(next_it, it_to);

        next_it = it_from;
        advance(next_it, 1);
        return next_it;
    }

    return it_to;
}

void filterSeqAlg1(tSequence &in_seq, size_t val) {

    auto it = in_seq.begin();
    bool outPrevPair = true;
    optional<long long> prev_y;
    size_t subseq_modlen = 0;
    tSequence::iterator prev_outit = it;

    size_t i = 0;

    for (; it != in_seq.end(); ++it) {
        const long long x = it->first, y = it->second;

        ++i;

        bool outCurPair = false;
        if (prev_y.has_value()) {
            if (prev_y == y) {
                ++subseq_modlen;
                if ((val-1) == subseq_modlen) {
                    outCurPair = true;
                    subseq_modlen = 0;
                }
            } else {
                subseq_modlen = 0;
                outCurPair = true;
                if (!outPrevPair) {
                      advance(it, -1);
                      it = delBetweenAndPosition(in_seq, prev_outit, it);
                      advance(it, 1);
                      prev_outit = it;
                }
            }
        } else {
           outCurPair = true;
        }

        if (outCurPair) {
            it = delBetweenAndPosition(in_seq, prev_outit, it);
            prev_outit = it;
        }

        prev_y = y;
        outPrevPair = outCurPair;
    }
    if (!outPrevPair) {
        advance(it, -1);
        it = delBetweenAndPosition(in_seq, prev_outit, it);
    }
}


void scanStr2Seq(const string &str_seq, tSequence &out_seq) {

    using It = sregex_iterator;
    optional<long long> prev_x;

    for (It it(str_seq.begin(), str_seq.cend(), rx); it != It(); ++it) {
        auto m = *it;

        long long x, y;
        const auto& s1 = m[1].str();
        const auto& s2 = m[2].str();

        from_chars(s1.data(), s1.data() + s1.size(), x, 10);
        from_chars(s2.data(), s2.data() + s2.size(), y, 10);
 
        assert(prev_x.value_or(x) <= x);
        out_seq.emplace_back(x, y);

        prev_x = x;
    }
}

void coutSeq(const tSequence &in_seq) {
    for (const auto& p : in_seq) {
        cout << " (" << p.first << ", " << p.second << ")";
    }
}

int main(int argc, char* argv[]) {

    assert(n > 2);

    cout << "Исходная: " << seq << endl;
    tSequence in_seq;
    scanStr2Seq(seq, in_seq);
    filterSeqAlg1(in_seq, n);

    cout << "Результат при n = " << n << ": ";
    coutSeq(in_seq);
    cout << endl;

    return 0;
}
