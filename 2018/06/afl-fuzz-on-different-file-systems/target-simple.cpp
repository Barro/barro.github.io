#define _POSIX_C_SOURCE 200809L

#include <cstdlib>
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

#ifdef USE_LIBFUZZER
std::map<char, int> g_values;

extern "C" int LLVMFuzzerTestOneInput(const char *data, size_t size) {
    if (size > 128) {
        size = 128;
    }
#ifdef USE_FMEMOPEN
    if (size == 0) {
        return 0;
    }
    char buffer[128] = "";
    FILE *data_fp = fmemopen((char*)data, size, "r");
    fread(buffer, 1, sizeof(buffer), data_fp);
    fclose(data_fp);
#endif // #ifdef USE_FMEMOPEN

    fuzz_one(data, size, &g_values);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    for (int i = 0; i < 'z' - 'a'; ++i) {
        g_values['a' + i] = i;
    }
    return 0;
}
#endif // #ifdef USE_LIBFUZZER

#ifndef USE_LIBFUZZER
int main(int argc, char* argv[])
{
    std::map<char, int> values;
    for (int i = 0; i < 'z' - 'a'; ++i) {
        values['a' + i] = i;
    }
    char buffer[128];

#if !defined(READ_FILENAME)
    int input_fd = fileno(stdin);
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
        int input_fd = open(argv[1], O_RDONLY);
#endif // #if defined(__READ_FILENAME)

        ssize_t read_size = read(input_fd, buffer, sizeof(buffer));
        if (read_size == -1) {
            abort();
        }
        fuzz_one(buffer, read_size, &values);

#if defined(READ_FILENAME)
        close(input_fd);
#endif // #if defined(__READ_FILENAME)
    }
#ifdef QUICK_EXIT
    std::_Exit(0);
#endif
    return 0;
}
#endif // #ifndef USE_LIBFUZZER
