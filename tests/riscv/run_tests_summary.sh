#!/bin/bash

# Run full test suite and capture summary

cd "$(dirname "$0")/../.."

echo "Running full RISC-V test suite..."
echo "This may take several minutes..."
echo ""

./tests/riscv/test_all_riscv.sh > /tmp/riscv_full_test.log 2>&1
EXIT_CODE=$?

echo "=========================================="
echo "TEST SUMMARY"
echo "=========================================="
echo ""

# Extract summary information
grep -E "Summary:|Total tests:|Passed:|Failed:" /tmp/riscv_full_test.log | tail -20

echo ""
echo "=========================================="
echo "FAILED TESTS (if any)"
echo "=========================================="
grep "^  FAIL:" /tmp/riscv_full_test.log | head -50

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "✅ All tests passed!"
else
    echo ""
    echo "❌ Some tests failed. See /tmp/riscv_full_test.log for details."
    echo ""
    echo "Total failures: $(grep -c '^  FAIL:' /tmp/riscv_full_test.log)"
fi

exit $EXIT_CODE

