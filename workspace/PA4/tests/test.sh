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

# Compare exit statuses
echo "Testing file: $test_file"
echo "----------------------------------------"

# Check exit status
if [ $mysemant_exit -eq $coolc_exit ]; then
  echo -e "${GREEN}✓ Exit status match: $mysemant_exit${NC}"
else
  echo -e "${RED}✗ Exit status mismatch: mysemant=$mysemant_exit, coolc=$coolc_exit${NC}"
fi

# Check stdout
if cmp -s $mysemant_out $coolc_out; then
  echo -e "${GREEN}✓ Standard output matches${NC}"
else
  echo -e "${RED}✗ Standard output differs${NC}"
  echo "Diff:"
  diff $mysemant_out $coolc_out
fi

# Check stderr
if cmp -s $mysemant_err $coolc_err; then
  echo -e "${GREEN}✓ Standard error matches${NC}"
else
  echo -e "${RED}✗ Standard error differs${NC}"
  echo "Diff:"
  diff $mysemant_err $coolc_err
fi

# Cleanup temp files
rm $mysemant_out $mysemant_err $coolc_out $coolc_err

exit 0
