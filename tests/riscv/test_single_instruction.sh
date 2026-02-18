#!/bin/bash
# Test single RISC-V instruction roundtrip: assemble -> disassemble -> compare

GOODASM="./build/goodasm"
TMPDIR="/tmp/riscv_test_$$"

mkdir -p "$TMPDIR"
trap "rm -rf $TMPDIR" EXIT

test_instruction() {
    local instr="$1"
    local name="$2"

    # Create assembly file
    echo ".lang riscv32" > "$TMPDIR/test.asm"
    echo ".org 0x0" >> "$TMPDIR/test.asm"
    echo "$instr" >> "$TMPDIR/test.asm"

    # Assemble
    if ! "$GOODASM" --riscv32 "$TMPDIR/test.asm" -o "$TMPDIR/test.bin" 2>"$TMPDIR/asm_err.txt"; then
        echo "FAIL [$name]: Assembly failed"
        cat "$TMPDIR/asm_err.txt"
        return 1
    fi

    # Check binary not empty
    if [ ! -s "$TMPDIR/test.bin" ]; then
        echo "FAIL [$name]: Empty binary"
        return 1
    fi

    # Disassemble
    local disasm
    disasm=$("$GOODASM" --riscv32 -d "$TMPDIR/test.bin" 2>"$TMPDIR/dis_err.txt")
    if [ $? -ne 0 ]; then
        echo "FAIL [$name]: Disassembly failed"
        cat "$TMPDIR/dis_err.txt"
        return 1
    fi

    # Normalize both strings for comparison
    local orig_norm=$(echo "$instr" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/^ *//' | sed 's/ *$//')
    local dis_norm=$(echo "$disasm" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/^ *//' | sed 's/ *$//')

    # Handle hex vs decimal immediate normalization
    # Convert hex #0x... to decimal for comparison
    orig_norm=$(echo "$orig_norm" | python3 -c "
import sys, re
for line in sys.stdin:
    def hex_to_dec(m):
        v = int(m.group(1), 16)
        if v >= 0x80000000: v -= 0x100000000
        return '#' + str(v)
    print(re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.I), end='')
" 2>/dev/null || echo "$orig_norm")

    dis_norm=$(echo "$dis_norm" | python3 -c "
import sys, re
for line in sys.stdin:
    def hex_to_dec(m):
        v = int(m.group(1), 16)
        if v >= 0x80000000: v -= 0x100000000
        return '#' + str(v)
    print(re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.I), end='')
" 2>/dev/null || echo "$dis_norm")

    if [ "$orig_norm" = "$dis_norm" ]; then
        echo "PASS [$name]: $instr"
        return 0
    else
        echo "FAIL [$name]: Mismatch"
        echo "  Original:     '$instr'"
        echo "  Disassembled: '$disasm'"
        echo "  Orig norm:    '$orig_norm'"
        echo "  Dis norm:     '$dis_norm'"
        return 1
    fi
}

# Run tests passed as arguments or from stdin
if [ $# -gt 0 ]; then
    test_instruction "$1" "${2:-test}"
else
    # Read instructions from stdin, one per line
    pass=0
    fail=0
    while IFS= read -r line; do
        if [ -n "$line" ] && [[ ! "$line" =~ ^# ]]; then
            if test_instruction "$line" "$line"; then
                ((pass++))
            else
                ((fail++))
            fi
        fi
    done
    echo "---"
    echo "Total: $((pass + fail)), Passed: $pass, Failed: $fail"
fi
