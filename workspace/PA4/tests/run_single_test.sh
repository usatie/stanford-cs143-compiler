#!/bin/bash

# Test script to compare output of ./mysemant against coolc
# Usage: ./test.sh [file.cl]

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ $# -eq 0 ]; then
  echo "Usage: $0 [file.cl]"
  echo "Example: $0 good.cl"
  exit 1
fi

test_file=$1

# Create temp files for outputs
mysemant_out=$(mktemp)
mysemant_err=$(mktemp)
coolc_out=$(mktemp)
coolc_err=$(mktemp)

# Run mysemant
./mysemant $test_file > $mysemant_out 2> $mysemant_err
mysemant_exit=$?

# Run coolc
coolc $test_file > $coolc_out 2> $coolc_err
coolc_exit=$?

# Initialize test status
test_passed=true

# Check exit status and store status
exit_status_match=true
if [ $mysemant_exit -ne $coolc_exit ]; then
  test_passed=false
  exit_status_match=false
fi

# Check stdout and store status
stdout_match=true
if ! cmp -s $mysemant_out $coolc_out; then
  test_passed=false
  stdout_match=false
fi

# Check stderr and store status
stderr_match=true
if ! cmp -s $mysemant_err $coolc_err; then
  test_passed=false
  stderr_match=false
fi

# If test failed, output error info to stderr
if [ "$test_passed" = false ]; then
  if [ "$exit_status_match" = false ]; then
    echo -e "${RED}Exit status mismatch${NC}: mysemant=$mysemant_exit, coolc=$coolc_exit" >&2
  fi
  
  if [ "$stdout_match" = false ]; then
    echo -e "${RED}Standard output differs${NC}" >&2
  fi
  
  if [ "$stderr_match" = false ]; then
    echo -e "${RED}Standard error differs${NC}:" >&2
    diff $mysemant_err $coolc_err >&2
  fi
fi

# Cleanup temp files
rm $mysemant_out $mysemant_err $coolc_out $coolc_err

if [ "$test_passed" = true ]; then
  exit 0
else
  exit 1
fi
