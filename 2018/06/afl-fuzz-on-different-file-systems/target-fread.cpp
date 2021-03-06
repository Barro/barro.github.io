#include <cstdlib>
#include <fstream>
#include <map>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define AFL_PERSISTENT_ITERATIONS 40000

void fuzz_one(const char* data, size_t size, const std::map<char, int>* values)
{
    long total = 0;
    for (size_t data_index = 0; data_index < size; ++data_index) {
        const char key = data[data_index];
        auto value_iterator = values->find(key);
        if (value_iterator == values->end()) {
            continue;
        }
        const char value = (*value_iterator).second;
        if (value % 5 == 0) {
            total += value * 5;
            total += key;
        } else if (value % 3 == 0) {
            total += value * 3;
            total += key;
        } else if (value % 2 == 0) {
            total += value * 2;
            total += key;
        } else {
            total += value + key;
        }
    }
    std::cout << total << "\n";
}

int main(int argc, char* argv[])
{
    std::map<char, int> values;
    for (int i = 0; i < 'z' - 'a'; ++i) {
        values['a' + i] = i;
    }
    char buffer[128];

#if !defined(READ_FILENAME)
    FILE* input_fd = stdin;
    setvbuf(input_fd, NULL, _IONBF, 0);
#endif // #if defined(__READ_FILENAME)

#if defined(__AFL_HAVE_MANUAL_CONTROL) && defined(AFL_DEFERRED)
    __AFL_INIT();
#endif // #if defined(__AFL_HAVE_MANUAL_CONTROL) && defined(AFL_DEFERRED)

#if defined(__AFL_LOOP) && defined(AFL_PERSISTENT)
    while (__AFL_LOOP(AFL_PERSISTENT_ITERATIONS)) {
#else // #if defined(__AFL_LOOP) && defined(AFL_PERSISTENT)
    bool iterating = true;
    while (iterating) {
        iterating = false;
#endif // #if defined(__AFL_LOOP) && defined(AFL_PERSISTENT)

#if defined(READ_FILENAME)
        FILE* input_fd = fopen(argv[1], "rb");
#else
        //rewind(input_fd);
#endif // #if defined(__READ_FILENAME)

        size_t read_size = fread(buffer, 1, sizeof(buffer), input_fd);
        fuzz_one(buffer, read_size, &values);

#if defined(READ_FILENAME)
        fclose(input_fd);
#endif // #if defined(__READ_FILENAME)
}
    std::_Exit(0);
    return 0;
}
