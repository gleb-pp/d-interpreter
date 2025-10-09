if [ -d .git ]; then
    DIR="src"
elif [ -d ../.git ]; then
    DIR="../src"
else
    echo "Run this script from the project's root directory OR one of its immediate subdirectories" >/dev/stderr
    exit 1
fi
TEMPFILE="$(mktemp)"
find "$DIR" -name '*.h' -o -name '*.cpp' >"$TEMPFILE"
clang-format -i --files="$TEMPFILE"
rm "$TEMPFILE"
