#!/bin/bash

BINKCC="./bin/kcc.exe"

CC=$(which clang++)
CC=$(which g++)
OPTS="-std=c++11 -g3"


function build() {
    sources=$(find . -type f -name "*.cc" -and -not -name "*_test.cc")
    ${CC} ${OPTS} ${sources} -o ${BINKCC}
}

function utest() {

    echo "##### begin tests #####"

    num_test=$(expr $(ls test/*_test.cc | wc -l))
    cnt=1

    for SRC in `ls test/*_test.cc`
    do
        executable="./test/bin/$(basename ${SRC##.cc})"
        sources=$(find . -type f -name "*.cc" -and -not -name "*_test.cc" -and -not -name "main.cc")
        ${CC} ${OPTS} ${SRC} ${sources} -o ${executable}

        if [[ $? == 0 ]]; then

            # run executable
            ${executable}

            if [[ $? == 0 ]]; then
                echo "SUCCESS (${cnt}/${num_test}) - ${SRC}"
            else
                echo "FAILED to run (${cnt}/${num_test}) - ${SRC}"
            fi

        else
            echo "FAILED (${cnt}/${num_test}) - ${SRC}"
        fi

        cnt=$(expr ${cnt} + 1)
    done

    echo "##### Finished all tests #####"
}

test -e ./bin || mkdir ./bin

if [ "$1" == "build" ]; then
    build
elif [ "$1" == "test" ]; then
    test -e ./test/bin || mkdir ./test/bin
    utest
elif [ "$1" == "all" ]; then
    build && utest
else
    echo "./cmd.sh (build|test|ast)"
fi
