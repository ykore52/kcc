#!/bin/bash

BINKCC="./bin/kcc"

function build() {
    sources=$(find . -type f -name "*.cc" -and -not -name "*_test.cc")
    g++ -std=c++11 ${sources} -o ${BINKCC}
}

function asttest() {
    g++ -std=c++11 ast_test.cc -o ./bin/ast
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

test -e "./bin" || mkdir ./bin

if [ "$1" == "build" ]; then
    build
elif [ "$1" == "test" ]; then
    unittest
elif [ "$1" == "ast" ]; then
	asttest
elif [ "$1" == "all" ]; then
    build && unittest
else
    echo "./cmd.sh (build|test|ast)"
fi