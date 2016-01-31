#!/bin/bash

set -euo pipefail

DIR=$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")
PYTHON_SRC=$DIR/target-simple.template.py
CXX_SRC=$DIR/target-simple.cpp
MEASUREMENT_TYPES=(
    dumb-mode
    pre-imports
    post-imports
    os-exit
    loop-seek
    afl-loop
)
rm -rf corpus-clean/ output-clean/
mkdir -p corpus-clean/ output-clean/
for sequence in a b abcdefgh; do
    echo -n "$sequence" > corpus-clean/"$sequence".txt
done

for python_version in python2 python3; do
    for measurement_type in "${MEASUREMENT_TYPES[@]}"; do
        grep -E "^(\$|([^#])|(#.+type-$measurement_type.*))" "$PYTHON_SRC" \
            | sed 's/^#//g' \
            > fuzz-target.py
        command=(
            py-afl-fuzz
            -S simple-1
            -T "$python_version"-"$measurement_type"
            -o output-clean/)
        if [[ "$measurement_type" == dumb-mode ]]; then
            command=(
                afl-fuzz -n
                -T "$python_version"-"$measurement_type"
                -o output-clean/simple-1/)
        fi
        AFL_SKIP_CPUFREQ=1 \
            /usr/bin/time timeout --foreground --signal=SIGINT 60 \
            "${command[@]}" -m 400 -i corpus-clean/ \
            -- /usr/bin/"$python_version" fuzz-target.py \
            2> cpu-time-"$python_version"-"$measurement_type".txt || :
        grep -E "^(start_time|last_update|execs_done|stability)" output-clean/simple-1/fuzzer_stats \
             > fuzzer-stats-"$python_version"-"$measurement_type".txt
    done
done

function run_cxx_target()
{
    local measurement_type=$1
    command=(
        afl-fuzz
        -S simple-1
        -T cxx-"$measurement_type"
        -o output-clean/)
    if [[ "$measurement_type" == dumb-mode ]]; then
        command=(
            afl-fuzz -n
            -T cxx-"$measurement_type"
            -o output-clean/simple-1/)
    fi
    AFL_SKIP_CPUFREQ=1 \
        /usr/bin/time timeout --foreground --signal=SIGINT 60 \
        "${command[@]}" -m 400 -i corpus-clean/ \
    -- ./fuzz-target \
    2> cpu-time-native-"$measurement_type".txt || :
    grep -E "^(start_time|last_update|execs_done|stability)" output-clean/simple-1/fuzzer_stats \
         > fuzzer-stats-native-"$measurement_type".txt
}

CXX_GENERIC_ARGS=(
    --std=c++11
    -o fuzz-target
    "$CXX_SRC"
)

afl-clang-fast++ "${CXX_GENERIC_ARGS[@]}"
run_cxx_target dumb-mode
run_cxx_target pre-imports
afl-clang-fast++ -DAFL_DEFERRED "${CXX_GENERIC_ARGS[@]}"
run_cxx_target post-imports
afl-clang-fast++ -DAFL_DEFERRED -DQUICK_EXIT "${CXX_GENERIC_ARGS[@]}"
run_cxx_target os-exit
afl-clang-fast++ -DAFL_PERSISTENT -DQUICK_EXIT "${CXX_GENERIC_ARGS[@]}"
run_cxx_target afl-loop
