#!/bin/bash

BENCH="./../../cmake-build-relwithdebinfo/lab1/extendible_bench_tests"

echo "=== Сбор метрик для малого N (10,000) ==="
perf stat -e cache-misses,LLC-load-misses $BENCH 10000 > /dev/null

echo -e "\n=== Сбор метрик для большого N (1,000,000) ==="
perf stat -e cache-misses,LLC-load-misses $BENCH 1000000 > /dev/null