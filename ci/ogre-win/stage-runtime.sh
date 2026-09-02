#!/bin/bash
# Stage a self-contained Windows runtime dir: starting from the exe and the
# OGRE plugin DLLs, walk PE import tables recursively and copy every
# non-system DLL found in the search dirs. DLLs not found in any search dir
# are assumed to be Windows system DLLs and left alone.
#
# Usage: stage-runtime.sh <runtime-dir> <root-files...> -- <search-dirs...>
set -euo pipefail

RUNTIME="$1"; shift
ROOTS=(); SEARCH=()
mode=roots
for a in "$@"; do
    if [ "$a" = "--" ]; then mode=search; continue; fi
    if [ $mode = roots ]; then ROOTS+=("$a"); else SEARCH+=("$a"); fi
done

mkdir -p "$RUNTIME"
declare -A seen=()
queue=("${ROOTS[@]}")

while [ ${#queue[@]} -gt 0 ]; do
    f="${queue[0]}"; queue=("${queue[@]:1}")
    [ -n "${seen[$f]:-}" ] && continue
    seen[$f]=1
    while read -r dll; do
        [ -z "$dll" ] && continue
        for d in "${SEARCH[@]}"; do
            if [ -f "$d/$dll" ]; then
                if [ ! -f "$RUNTIME/$dll" ]; then
                    echo "  staging $dll (from $d)"
                    cp "$d/$dll" "$RUNTIME/"
                fi
                queue+=("$RUNTIME/$dll")
                break
            fi
        done
    done < <(objdump -p "$f" 2>/dev/null | grep -oE "DLL Name: .*" | sed 's/DLL Name: //')
done

echo "staged $(ls "$RUNTIME" | wc -l) files in $RUNTIME"
