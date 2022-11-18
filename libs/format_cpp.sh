#!/bin/sh
git ls-files | grep -E "\.cpp|\.h" | grep -v "3rdparty" | xargs -n1 clang-format -i
