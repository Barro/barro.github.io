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

    // std::ifstream operations change global state on the first
    // access. These calls here make the stability number of afl-fuzz
    // to go to 100 %, as this state is not modified inside fuzzing
    // blocks:
    {
        std::ifstream input_fd_start(
            "/dev/zero", std::ifstream::in | std::ifstream::binary);
        input_fd_start.read(buffer, sizeof(buffer));
        input_fd_start.seekg(0, input_fd_start.beg);
        input_fd_start.clear();
        input_fd_start.gcount();
        input_fd_start.close();
    }

#if !defined(READ_FILENAME)
    auto& input_fd = std::cin;
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
        std::ifstream input_fd(
            argv[1], std::ifstream::in | std::ifstream::binary);
#else
        input_fd.seekg(0, input_fd.beg);
        input_fd.clear();
#endif // #if defined(__READ_FILENAME)

        input_fd.read(buffer, sizeof(buffer));
        size_t read_size = input_fd.gcount();
        fuzz_one(buffer, read_size, &values);

#if defined(READ_FILENAME)
        input_fd.close();
#endif // #if defined(__READ_FILENAME)
    }
    std::_Exit(0);
    return 0;
}
