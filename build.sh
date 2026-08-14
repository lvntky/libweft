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
  fmt         run clang-format over source/include/example
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

case "${1:-}" in
    configure|build|test|example|install|clean|fmt|tidy|amalgamate)
        cmd=$1; shift ;;
    -h|--help) usage; exit 0 ;;
    -*) cmd=build ;;
    "") cmd=build ;;
    *) printf 'unknown command: %s\n\n' "$1" >&2; usage >&2; exit 2 ;;
esac

while [ $# -gt 0 ]; do
    case $1 in
        -t|--type)    type=$2; shift 2 ;;
        -b|--dir)     dir=$2; shift 2 ;;
        -p|--prefix)  prefix=$2; shift 2 ;;
        -j|--jobs)    jobs=$2; shift 2 ;;
        -s|--shared)  shared=ON; shift ;;
        -d|--dev)     dev=ON; shift ;;
        -v|--verbose) verbose="--verbose"; shift ;;
        -h|--help)    usage; exit 0 ;;
        *) printf 'unknown option: %s\n' "$1" >&2; exit 2 ;;
    esac
done

[ -n "$dir" ] || dir="build/$type"

sanitize=""
cmake_type=$type
case $type in
    Asan) cmake_type=Debug; sanitize="address,undefined" ;;
    Tsan) cmake_type=Debug; sanitize="thread" ;;
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
    [ -e compile_commands.json ] || ln -sf "$dir/compile_commands.json" .
}

need_configure() {
    [ ! -f "$dir/CMakeCache.txt" ]
}

do_build() {
    need_configure && configure
    cmake --build "$dir" -j "$jobs" $verbose
}

case $cmd in
    configure) configure ;;
    build)     do_build ;;
    test)      dev=ON; need_configure && configure
               cmake --build "$dir" -j "$jobs" $verbose
               ctest --test-dir "$dir" --output-on-failure ;;
    example)   do_build; "$dir/example/echo" ;;
    install)   do_build; cmake --install "$dir" ;;
    clean)     rm -rf "$dir" compile_commands.json ;;
    fmt)       find source include example -name '*.[ch]' -print0 \
                 | xargs -0 clang-format -i ;;
    tidy)      need_configure && configure
               find source -name '*.c' -print0 \
                 | xargs -0 clang-tidy -p "$dir" ;;
    amalgamate)
               python3 tools/amalgamate.py -o "$dir/weft-single" ;;
esac
