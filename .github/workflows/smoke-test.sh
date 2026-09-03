#!/usr/bin/env bash
# Headless smoke test for citygen.
# Usage: .github/workflows/smoke-test.sh <runtime-dir>
# Runs the app under Xvfb (software GL), lets the render loop spin briefly,
# then checks the Ogre log for: Cg plugin installed, resources parsed,
# no escaped exceptions.
set -euo pipefail

RUNTIME_DIR="${1:?usage: .github/workflows/smoke-test.sh <runtime-dir>}"
cd "$RUNTIME_DIR"

rm -f citygen.log

# The app is a GUI event loop: it never exits on its own, so timeout(1)
# terminates it. 124 = killed by timeout = the loop ran without crashing.
set +e
GDK_BACKEND=x11 xvfb-run -a -s "-screen 0 1280x1024x24" timeout 45 ./citygen
code=$?
set -e

echo "citygen exit code: $code (124 = terminated by timeout, expected)"

if [ "$code" -ne 124 ] && [ "$code" -ne 0 ]; then
    echo "FAIL: citygen exited abnormally before the timeout"
    tail -30 citygen.log || true
    exit 1
fi

grep -q "Installing plugin: Cg Program Manager" citygen.log \
    || { echo "FAIL: Cg Program Manager plugin was not installed"; tail -30 citygen.log; exit 1; }

grep -q "Finished parsing scripts for resource group General" citygen.log \
    || { echo "FAIL: material scripts were not parsed"; tail -30 citygen.log; exit 1; }

if grep -qE "Unhandled unknown exception|OGRE EXCEPTION" citygen.log; then
    echo "FAIL: exceptions in Ogre log"
    grep -E "Unhandled unknown exception|OGRE EXCEPTION" citygen.log | head -5
    exit 1
fi

echo "SMOKE TEST PASSED"
