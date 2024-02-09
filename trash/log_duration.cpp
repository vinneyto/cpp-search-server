#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

vector<int> ReverseVector(const vector<int>& source_vector) {
    vector<int> res;
    for (int i : source_vector) {
        res.insert(res.begin(), i);
    }

    return res;
}

int CountPops(const vector<int>& source_vector, int begin, int end) {
    int res = 0;

    for (int i = begin; i < end; ++i) {
        if (source_vector[i]) {
            ++res;
        }
    }

    return res;
}

void AppendRandom(vector<int>& v, int n) {
    for (int i = 0; i < n; ++i) {
        // получаем случайное число с помощью функции rand.
        // с помощью (rand() % 2) получим целое число в диапазоне 0..1.
        // в C++ имеются более современные генераторы случайных чисел,
        // но в данном уроке не будем их касаться
        v.push_back(rand() % 2);
    }
}

#include <chrono>
#include <iostream>

class LogDuration {
   public:
    // заменим имя типа std::chrono::steady_clock
    // с помощью using для удобства
    using Clock = std::chrono::steady_clock;

    LogDuration(string name) : name_(name) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        std::cerr << name_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

   private:
    const Clock::time_point start_time_ = Clock::now();
    const string name_;
};

class StreamUntier {
   public:
    StreamUntier(istream& is) : is_(is) {
        tied_before_ = is.tie(nullptr);
    }

    ~StreamUntier() {
        is_.tie(tied_before_);
    }

   private:
    istream& is_;
    ostream* tied_before_;
};

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

void Operate() {
    vector<int> random_bits;

    // операция << для целых чисел это сдвиг всех бит в двоичной
    // записи числа. Запишем с её помощью число 2 в степени 17 (131072)
    static const int N = 1 << 17;

    {
        LOG_DURATION("Append random");

        // заполним вектор случайными числами 0 и 1
        AppendRandom(random_bits, N);
    }

    vector<int> reversed_bits;

    {
        LOG_DURATION("Reverse");

        // перевернём вектор задом наперёд
        reversed_bits = ReverseVector(random_bits);
    }

    {
        LOG_DURATION("Counting");

        // посчитаем процент единиц на начальных отрезках вектора
        for (int i = 1, step = 1; i <= N; i += step, step *= 2) {
            // чтобы вычислить проценты, мы умножаем на литерал 100. типа double;
            // целочисленное значение функции CountPops при этом автоматически
            // преобразуется к double, как и i
            double rate = CountPops(reversed_bits, 0, i) * 100. / i;
            cout << "After "s << i << " bits we found "s << rate << "% pops"s
                 << endl;
        }
    }
}

int main() {
    Operate();
    return 0;
}