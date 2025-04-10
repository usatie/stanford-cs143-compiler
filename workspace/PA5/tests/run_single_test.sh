#!/bin/bash

# Test script to compare output of ./mycoolc against coolc
# Usage: ./test.sh [file.cl]

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ $# -eq 0 ]; then
  echo "Usage: $0 [file.cl]"
  echo "Example: $0 example.cl"
  exit 1
fi

test_file=$1

# Create temp files for outputs
mycoolc_out=$(mktemp)
mycoolc_err=$(mktemp)
coolc_out=$(mktemp)
coolc_err=$(mktemp)

# Create temp files for assembly output
mycoolc_s="${test_file%.cl}.mycoolc.s"
coolc_s="${test_file%.cl}.coolc.s"

# Run mycoolc
./mycoolc $test_file > $mycoolc_out 2> $mycoolc_err
mycoolc_exit=$?

# If compilation succeeded, move the .s file
if [ -f "${test_file%.cl}.s" ]; then
  mv "${test_file%.cl}.s" "$mycoolc_s"
fi

# Run coolc
coolc $test_file > $coolc_out 2> $coolc_err
coolc_exit=$?

# If compilation succeeded, rename the coolc .s file
if [ -f "${test_file%.cl}.s" ]; then
  mv "${test_file%.cl}.s" "$coolc_s"
fi

# Initialize test status
test_passed=true

# Check exit status and store status
exit_status_match=true
if [ $mycoolc_exit -ne $coolc_exit ]; then
  test_passed=false
  exit_status_match=false
fi

# Check stdout and store status
stdout_match=true
if ! cmp -s $mycoolc_out $coolc_out; then
  test_passed=false
  stdout_match=false
fi

# Check stderr and store status
stderr_match=true
if ! cmp -s $mycoolc_err $coolc_err; then
  test_passed=false
  stderr_match=false
fi

# Check assembly output if both compilers succeeded
assembly_match=true
if [ $mycoolc_exit -eq 0 ] && [ $coolc_exit -eq 0 ]; then
  if [ -f "$mycoolc_s" ] && [ -f "$coolc_s" ]; then
    # Compare assembly files but ignore specific differences (like comments or labels)
    # This is a simple diff - you might need a more sophisticated comparison
    if ! diff -I '^#.*' -I '^\s*\..*' "$mycoolc_s" "$coolc_s" > /dev/null; then
      test_passed=false
      assembly_match=false
    fi
  else
    echo -e "${RED}Missing assembly output${NC}: mycoolc_s=$mycoolc_s, coolc_s=$coolc_s" >&2
    test_passed=false
    assembly_match=false
  fi
fi

# If test failed, output error info to stderr
if [ "$test_passed" = false ]; then
  if [ "$exit_status_match" = false ]; then
    echo -e "${RED}Exit status mismatch${NC}: mycoolc=$mycoolc_exit, coolc=$coolc_exit" >&2
  fi
  
  if [ "$stdout_match" = false ]; then
    echo -e "${RED}Standard output differs${NC}" >&2
    diff $mycoolc_out $coolc_out | head -20 >&2
  fi
  
  if [ "$stderr_match" = false ]; then
    echo -e "${RED}Standard error differs${NC}:" >&2
    diff $mycoolc_err $coolc_err | head -20 >&2
  fi
  
  if [ "$assembly_match" = false ] && [ -f "$mycoolc_s" ] && [ -f "$coolc_s" ]; then
    echo -e "${RED}Assembly output differs${NC}:" >&2
    echo "Check the assembly files for detailed differences:" >&2
    echo "  - $mycoolc_s" >&2
    echo "  - $coolc_s" >&2
  fi
fi

# Cleanup temp files
rm $mycoolc_out $mycoolc_err $coolc_out $coolc_err

# Create test-diff directory if it doesn't exist
mkdir -p $(dirname "$0")/test-diff

# Handle assembly files based on test results
if [ "$test_passed" = true ]; then
  # Test passed, remove assembly files
  if [ -f "$mycoolc_s" ]; then
    rm "$mycoolc_s"
  fi
  if [ -f "$coolc_s" ]; then
    rm "$coolc_s"
  fi
else
  # Test failed with assembly differences, move files to test-diff folder
  if [ "$assembly_match" = false ] && [ -f "$mycoolc_s" ] && [ -f "$coolc_s" ]; then
    base_filename=$(basename "${test_file%.cl}")
    mv "$mycoolc_s" "$(dirname "$0")/test-diff/${base_filename}.mycoolc.s"
    mv "$coolc_s" "$(dirname "$0")/test-diff/${base_filename}.coolc.s"
    echo -e "Assembly files saved to test-diff directory for inspection"
  else
    # Clean up assembly files that weren't problematic
    if [ -f "$mycoolc_s" ]; then
      rm "$mycoolc_s"
    fi
    if [ -f "$coolc_s" ]; then
      rm "$coolc_s"
    fi
  fi
fi

# Exit with appropriate status
if [ "$test_passed" = true ]; then
  exit 0
else
  exit 1
fi