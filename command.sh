#!/bin/bash

BINKCC="./bin/kcc"

function build() {
    gcc -std=c++11 main.cpp -lstdc++ -o ${BINKCC}
}

function unittest() {
    ${BINKCC} return2.c || echo "FAIL"

    tmpfile=`mktemp`
    { \
        echo "int main() {}"; \
    } >> ${tmpfile}
    ${BINKCC} ${tmpfile}
    if [[ $? == 0 ]]; then
        echo "OK"
    else 
        echo "FAIL"
    fi
    rm ${tmpfile}

    echo "===== finish unit tests ====="
}

if [ "$1" == "build" ]; then
    build
elif [ "$1" == "test" ]; then
    unittest
fi