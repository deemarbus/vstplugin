#!/usr/bin/env bash
set -u

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

printf 'QuadTable local build diagnosis\n'
printf '===============================\n'
printf 'Repo: %s\n' "$repo_root"
printf '\nTool versions:\n'
command -v cmake >/dev/null 2>&1 && cmake --version | head -1 || printf 'cmake: NOT FOUND\n'
command -v c++ >/dev/null 2>&1 && c++ --version | head -1 || printf 'c++: NOT FOUND\n'
command -v git >/dev/null 2>&1 && git --version || printf 'git: NOT FOUND\n'

printf '\nJUCE candidates:\n'
candidates=()
[ -n "${JUCE_SOURCE_DIR:-}" ] && candidates+=("$JUCE_SOURCE_DIR")
candidates+=("$repo_root/JUCE" "$repo_root/juce" "$repo_root/../JUCE" "$HOME/dev/JUCE")

found=0
for candidate in "${candidates[@]}"; do
    if [ -f "$candidate/CMakeLists.txt" ] && [ -d "$candidate/modules/juce_audio_processors" ]; then
        printf 'FOUND: %s\n' "$candidate"
        found=1
    else
        printf 'missing: %s\n' "$candidate"
    fi
done

printf '\nRecommended commands:\n'
if [ "$found" -eq 1 ]; then
    printf 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DQUADTABLE_REQUIRE_JUCE=ON\n'
    printf 'cmake --build build --config Release\n'
else
    printf 'No JUCE checkout was found. For the core-only smoke test, run:\n'
    printf 'cmake -S . -B build -DCMAKE_BUILD_TYPE=Release\n'
    printf 'cmake --build build --config Release\n'
    printf 'ctest --test-dir build --output-on-failure\n'
    printf '\nTo build the plugin UI, put JUCE in one of these folders or pass -DJUCE_SOURCE_DIR=/path/to/JUCE.\n'
fi
