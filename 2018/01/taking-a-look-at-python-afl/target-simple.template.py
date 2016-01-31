"""
This can be run with a following command after afl
<http://lcamtuf.coredump.cx/afl/> and python-afl
<https://github.com/jwilk/python-afl> have been installed:

    AFL_SKIP_CPUFREQ=1 py-afl-fuzz -S simple-1 -m 200 -i input/ -o output/ -- \
        /usr/bin/python target-python-simple.py

"""

#import afl  # type-pre-imports type-post-imports type-os-exit type-afl-loop type-loop-seek
#afl.init()  # type-pre-imports
import os
import string
import sys

try:
    stdin_compat = sys.stdin.buffer
except AttributeError:
    stdin_compat = sys.stdin

#try:  # type-loop-seek
#    stdin_compat.seek(0)  # type-loop-seek

#    def seek_stdin():  # type-loop-seek
#        stdin_compat.seek(0)  # type-loop-seek
#except (IOError, OSError):  # type-loop-seek
#    def seek_stdin():  # type-loop-seek
#        pass  # type-loop-seek


VALUES = {}
for value, char in enumerate(string.ascii_lowercase):
    VALUES[char.encode("utf-8")] = value


def fuzz_one(stdin, values):
    data = stdin.read(128)
    total = 0
    for key in data:
        if key not in values:
            continue
        value = values[key]
        if value % 5 == 0:
            total += value * 5
            total += ord(key)
        elif value % 3 == 0:
            total += value * 3
            total += ord(key)
        elif value % 2 == 0:
            total += value * 2
            total += ord(key)
        else:
            total += value + ord(key)
    print(total)


#afl.init()  # type-post-imports type-os-exit
#fuzz_one(stdin_compat, VALUES)  # type-os-exit
#while afl.loop(10000):  # type-afl-loop type-loop-seek
#    seek_stdin()  # type-loop-seek
#    fuzz_one(stdin_compat, VALUES)  # type-afl-loop type-loop-seek
#os._exit(0)  # type-afl-loop type-os-exit
fuzz_one(stdin_compat, VALUES)
