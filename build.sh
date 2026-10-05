#!/usr/bin/env bash
# build.sh

set -euo pipefail

CC="clang"
CXX="clang++"
AS="clang"

TARGET="aarch64-unknown-windows"
SRC_DIR="src"

BIN_NAME="paxon.efi"

pkill qemu-system-aar || true

COMMON_FLAGS=(
    --target=$TARGET
    -ffreestanding
    -Wall
    -Wextra
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -Werror
    -g -gdwarf
    
    -Wno-unused-variable
    -Wno-c++17-extensions

    -Iincludes
    -Iincludes/CSL
    -Iincludes/CSL/specific-includes
    -Iincludes/uefi-headers
    -Iincludes/uefi-headers/AArch64
)

CFLAGS=(
    "${COMMON_FLAGS[@]}"
    --std=c23
    -fno-stack-protector
    -MMD
    -MP
)

CPPFLAGS=(
    "${COMMON_FLAGS[@]}"
    -fno-exceptions
    -fno-rtti
    -fshort-wchar
    -MMD
    -MP
    -nostdinc++
    --std=c++26
)

ASFLAGS=(
    -target "$TARGET"
)

LD=(
    ld.lld
    -m arm64pe
    --entry=csl_bootstrap
)


rm -rf build
mkdir build

find $SRC_DIR -type f | while read -r file; do
    rel="${file#src/}"
    obj="build/${rel%.*}.o"

    mkdir -p "$(dirname "$obj")"

    case "$file" in
        *.c)
            echo "[CC] $file"
            "$CC" "${CFLAGS[@]}" -c "$file" -o "$obj"
            ;;
        *.cpp)
            echo "[CXX] $file"
            "$CXX" "${CPPFLAGS[@]}" -c "$file" -o "$obj"
            ;;
        *.s|*.S)
            echo "[AS] $file"
            "$AS" "${ASFLAGS[@]}" -c "$file" -o "$obj"
            ;;
    esac
done

echo "[LD] $BIN_NAME"

OBJS=()

while IFS= read -r -d '' obj; do
    OBJS+=("$obj")
done < <(find build -type f -name '*.o' -print0)

"${LD[@]}" "${OBJS[@]}" -o $BIN_NAME

echo "[UEFI] Patching PE subsystem → EFI_APPLICATION"

python3 - "$PWD/$BIN_NAME" <<'PY'
import sys
import struct

path = sys.argv[1]

with open(path, "r+b") as f:
    # DOS header → PE header
    f.seek(0x3c)
    pe_offset = struct.unpack("<I", f.read(4))[0]

    # PE signature + COFF header
    f.seek(pe_offset + 4)
    machine, sections, timestamp, symptr, symbols, opt_size, characteristics = \
        struct.unpack("<HHIIIHH", f.read(20))

    # PE32+ Optional Header:
    # Subsystem is at offset 68 (0x44)
    subsystem_offset = pe_offset + 4 + 20 + 0x44

    f.seek(subsystem_offset)
    old = struct.unpack("<H", f.read(2))[0]

    print(f"[UEFI] Subsystem: 0x{old:X} → 0xA")

    f.seek(subsystem_offset)
    f.write(struct.pack("<H", 0xA))
PY

cp -v $BIN_NAME esp/EFI/BOOT/BOOTAA64.efi

llvm-readobj --coff-basereloc $BIN_NAME