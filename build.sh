#!/bin/bash

DEBUG=FALSE
DEBUG_OPTION=""
DEBUG_NAME=""
if [[ $1 == "-d" ]]; then
  DEBUG=TRUE
  DEBUG_OPTION="-DCMAKE_BUILD_TYPE=Debug"
  DEBUG_NAME="-debug"
fi

RAMULATOR2_DIR="/path/to/AiMulator"
RAMULATOR2_BUILD_DIR="${RAMULATOR2_DIR}/build${DEBUG_NAME}"
RAMULATOR2_CMAKE_DIR=${RAMULATOR2_DIR}/ext
RAMULATOR2_NAME="ramulator2"
TEST_FILE="${RAMULATOR2_DIR}/test/aim/test_aim.yaml"

cd ${RAMULATOR2_DIR}

if [[ -d ${RAMULATOR2_BUILD_DIR} ]]; then
  rm -rf ${RAMULATOR2_BUILD_DIR}
fi
if [[ -f ${} ]]; then
  rm -f "./${RAMULATOR2_NAME}${DEBUG_NAME}"
fi

mkdir ${RAMULATOR2_BUILD_DIR} && cd ${RAMULATOR2_BUILD_DIR}
cmake ${DEBUG_OPTION} ..
make -j
if [[ -f "./${RAMULATOR2_NAME}${DEBUG_NAME}" ]]; then
  cp "./${RAMULATOR2_NAME}${DEBUG_NAME}" "../${RAMULATOR2_NAME}${DEBUG_NAME}"
else
  echo "Error: Build failed. Binary not found."
  exit 1
fi

cd ${RAMULATOR2_DIR}
if [[ ${DEBUG} == TRUE ]]; then
  echo "Build for gdb debugging done."
else
  echo "Running ${TEST_FILE}..."
  ./${RAMULATOR2_NAME}${DEBUG_NAME} -f ${TEST_FILE}
fi