#!/bin/bash

# Run tests on all .cl files in tests directory
# Usage: ./run_all_tests.sh

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Get directory where script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Find all .cl files in tests directory
echo -e "${YELLOW}Running tests on all .cl files in tests directory...${NC}"
echo "----------------------------------------"

# Counter for tests
passed=0
failed=0
total=0

# Test files in tests directory
for test_file in "$SCRIPT_DIR"/*.cl; do
  if [ -f "$test_file" ]; then
    # Get file basename for cleaner output
    filename=$(basename "$test_file")
    
    # Run the test script and capture output
    output=$("$SCRIPT_DIR/run_single_test.sh" "$test_file" 2>&1)
    
    # Check result
    result=$?
    if [ $result -eq 0 ]; then
      echo -e "${GREEN}✓ PASS${NC}: $filename"
      passed=$((passed + 1))
    else
      echo -e "${RED}✗ FAIL${NC}: $filename"
      if [ -n "$output" ]; then
        echo "$output"
        echo "--------------------------"
      fi
      failed=$((failed + 1))
    fi
    total=$((total + 1))
  fi
done

# Print summary
echo -e "\n----------------------------------------"
echo -e "${YELLOW}Test Summary:${NC}"
echo -e "Passed: ${GREEN}$passed${NC}"
echo -e "Failed: ${RED}$failed${NC}"
echo -e "Total: $total"

if [ $failed -eq 0 ]; then
  echo -e "${GREEN}All tests passed!${NC}"
  exit 0
else
  echo -e "${RED}Some tests failed.${NC}"
  exit 1
fi
