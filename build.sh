#!/bin/sh
set -eu

usage() {
    cat <<'EOF'
usage: ./build.sh [command] [options]

commands:
  configure   run cmake configure step
  build       build the library (default)
  test        build and run tests
  example     build and run the echo example
  install     install to --prefix
  clean       remove the build directory
  fmt         run clang-format over source/include/example/test
  tidy        run clang-tidy
  amalgamate  generate single-file weft.h / weft.c

options:
  -t, --type TYPE      Debug|Release|RelWithDebInfo|Asan|Tsan   (default: Debug)
  -b, --dir DIR        build directory                          (default: build/<type>)
  -p, --prefix DIR     install prefix                           (default: /usr/local)
  -s, --shared         build shared library                     (default: static)
  -j, --jobs N         parallel jobs                            (default: nproc)
  -d, --dev            enable developer mode
  -v, --verbose        verbose build output
  -h, --help           show this

commands and options may appear in any order.
EOF
}

cmd=""
type=Debug
dir=""
prefix=/usr/local
shared=OFF
jobs=$(command -v nproc >/dev/null 2>&1 && nproc || echo 4)
dev=OFF
verbose=""

while [ $# -gt 0 ]; do
    case $1 in
        configure|build|test|example|install|clean|fmt|tidy|amalgamate)
            if [ -n "$cmd" ]; then
                printf 'multiple commands given: %s and %s\n' "$cmd" "$1" >&2
                exit 2
            fi
            cmd=$1
            shift
            ;;
        -t|--type)
            [ $# -ge 2 ] || { printf '%s requires an argument\n' "$1" >&2; exit 2; }
            type=$2; shift 2 ;;
        -b|--dir)
            [ $# -ge 2 ] || { printf '%s requires an argument\n' "$1" >&2; exit 2; }
            dir=$2; shift 2 ;;
        -p|--prefix)
            [ $# -ge 2 ] || { printf '%s requires an argument\n' "$1" >&2; exit 2; }
            prefix=$2; shift 2 ;;
        -j|--jobs)
            [ $# -ge 2 ] || { printf '%s requires an argument\n' "$1" >&2; exit 2; }
            jobs=$2; shift 2 ;;
        -s|--shared)  shared=ON; shift ;;
        -d|--dev)     dev=ON; shift ;;
        -v|--verbose) verbose=--verbose; shift ;;
        -h|--help)    usage; exit 0 ;;
        --)           shift; break ;;
        *)
            printf 'unknown argument: %s\n\n' "$1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

[ -n "$cmd" ] || cmd=build
[ -n "$dir" ] || dir="build/$type"

sanitize=""
cmake_type=$type
case $type in
    Debug|Release|RelWithDebInfo|MinSizeRel) ;;
    Asan) cmake_type=Debug; sanitize=address,undefined ;;
    Tsan) cmake_type=Debug; sanitize=thread ;;
    *) printf 'unknown build type: %s\n' "$type" >&2; exit 2 ;;
esac

configure() {
    set -- \
        -S . -B "$dir" \
        -DCMAKE_BUILD_TYPE="$cmake_type" \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -DBUILD_SHARED_LIBS="$shared" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -Dweft_DEVELOPER_MODE="$dev"

    if command -v ninja >/dev/null 2>&1; then
        set -- "$@" -G Ninja
    fi

    if [ -n "$sanitize" ]; then
        set -- "$@" \
            -DCMAKE_C_FLAGS="-fsanitize=$sanitize -fno-omit-frame-pointer -g" \
            -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=$sanitize" \
            -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=$sanitize"
    fi

    cmake "$@"
    ln -sf "$dir/compile_commands.json" compile_commands.json 2>/dev/null || true
}

ensure_configured() {
    [ -f "$dir/CMakeCache.txt" ] || configure
}

do_build() {
    ensure_configured
    if [ -n "$verbose" ]; then
        cmake --build "$dir" -j "$jobs" --verbose
    else
        cmake --build "$dir" -j "$jobs"
    fi
}

sources() {
    find source include example test \
        \( -name '*.c' -o -name '*.h' \) \
        ! -name '*.h.in' \
        -print0 2>/dev/null
}

case $cmd in
    configure)
        configure
        ;;
    build)
        do_build
        ;;
    test)
        dev=ON
        ensure_configured
        do_build
        ctest --test-dir "$dir" --output-on-failure
        ;;
    example)
        do_build
        "$dir/example/echo"
        ;;
    install)
        do_build
        cmake --install "$dir"
        ;;
    clean)
        rm -rf "$dir"
        [ -L compile_commands.json ] && rm -f compile_commands.json
        ;;
    fmt)
        sources | xargs -0 "${CLANG_FORMAT:-clang-format}" -i
        ;;
    tidy)
        ensure_configured
        find source -name '*.c' -print0 \
            | xargs -0 "${CLANG_TIDY:-clang-tidy}" -p "$dir"
        ;;
    amalgamate)
        python3 tools/amalgamate.py -o "$dir/weft-single"
        ;;
esac
