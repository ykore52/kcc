#!/bin/bash

BINKCC="./bin/kcc.exe"

CC=$(which clang++)
CC=$(which g++)
OPTS="-std=c++11 -g3"


function build() {
    sources=$(find . -type f -name "*.cc" -and -not -name "*_test.cc")
    ${CC} ${OPTS} ${sources} -o ${BINKCC}
}

function unittest() {
    echo "===== start unit tests ====="


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

    ${BINKCC} test/return2.c || echo "FAIL"

    # tmpfile=`mktemp`
    # { \
    #     echo "int main() {"; \
    #     echo "        return 2;" \
    #     echo "}"; \
    # } >> ${tmpfile}
    # ${BINKCC} ${tmpfile}
    # if [[ $? == 0 ]]; then
    #     echo "OK"
    # else 
    #     echo "FAIL"
    # fi
    # rm ${tmpfile}

    echo "===== finish unit tests ====="
}

test -e "./bin" || mkdir ./bin

if [ "$1" == "build" ]; then
    build
elif [ "$1" == "test" ]; then
    unittest
elif [ "$1" == "all" ]; then
    build && unittest
else
    echo "./cmd.sh (build|test|ast)"
fi
