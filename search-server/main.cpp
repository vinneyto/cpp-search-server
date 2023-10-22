// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:

// Закомитьте изменения и отправьте их в свой репозиторий.
#include <iostream>
#include <string>

using namespace std;

int main() {
    int k = 0;

    for (int i = 0; i <= 1000; i++) {
        const string str = to_string(i);
        for (const char &digit : str) {
            if (digit == '3') {
                ++k;
                break;
            }
        }
    }

    cout << k << endl;  // 271 не вините строго, я был уставший и мне было так влом думать

    // а еще это у меня повтор курса. репа старая (я думал заново пересоздается). я сделал hard reset force

    return 0;
}
