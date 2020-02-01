#!/usr/bin/env bash
rm -rf build

CMAKE_VARS="-DCMAKE_CXX_OUTPUT_EXTENSION_REPLACE=ON"

if [[ "ENABLE_COVERAGE" == "ON" ]];
then
    CMAKE_VARS="${CMAKE_VARS} -DENABLE_COVERAGE=ON"
fi

if [[ "$GTEST_LATEST" == "ON" ]];
then
    mkdir build
    cd build

    CMAKE_VARS="${CMAKE_VARS} -DGTEST_LATEST=ON"
else
    GTEST=googletest-release-1.8.1
    if [[ -e ${GTEST} ]]
    then
        echo "${GTEST} has already been unzipped, skip."
    else
        echo "Begin to unzip ${GTEST}.zip"
        unzip ${GTEST}.zip
        echo "Unzip google test finish"
    fi

    mkdir build
    cd build

    # Make Google Test in './build/gtest/' folder
    mkdir gtest
    cd gtest
    cmake ../../${GTEST}
    make
    cd ..
fi

# Make whole project
cmake ${CMAKE_VARS} ../
make

# Run Test
./pull-stream/test/pull-stream-test

cd ../

if [[ "ENABLE_COVERAGE" == "ON" ]];
then
    COVERAGE_FILE=coverage.info
    REPORT_FOLDER=coverage_report
    lcov --rc lcov_branch_coverage=1 -c -d build -o ${COVERAGE_FILE}_tmp
    lcov --rc lcov_branch_coverage=1  -e ${COVERAGE_FILE}_tmp "*pull-stream*" -o ${COVERAGE_FILE}
    genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${REPORT_FOLDER}
    rm -rf ${COVERAGE_FILE}_tmp
    rm -rf ${COVERAGE_FILE}

    if [[ "$OSTYPE" == "darwin"* ]]; then
        open ./${REPORT_FOLDER}/index.html
    fi
fi
