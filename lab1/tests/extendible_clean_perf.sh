#!/bin/bash

BINARY="./../../cmake-build-relwithdebinfo/lab1/extendible_perf_tests"
N=1000000

run_clean_test() {
    local OP=$1
    echo "--- Тестируем $OP ---"

    $BINARY $N INSERT

    perf record -g --call-graph dwarf -o "perf_reports/perf_${OP}.data" $BINARY $N $OP
    echo "Отчет сохранен в perf_reports/perf_${OP}.data"
    echo "------------------------"
}

for OPERATION in "GET" "UPDATE" "DELETE" "INSERT"
do
    run_clean_test $OPERATION
done
