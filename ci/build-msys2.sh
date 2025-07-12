#!/bin/bash -e

. ./ci/build-common.sh

args=(
  -D{cdda,d3d-hwaccel,d3d11,dvdnav,jpeg,lcms2,libarchive}=enabled
  -D{libbluray,lua,shaderc,spirv-cross,uchardet,vapoursynth}=enabled
  -D{egl-angle-lib,egl-angle-win32,pdf-build,rubberband,win32-smtc}=enabled
)

if [[ "$SYS" == "clang64" ]]; then
    args+=(
      -Db_sanitize=address,undefined
    )
else # sanitizers are not supported on stable rust yet so clang64 is excluded
    echo "::group::Building subrandr"
    build_subrandr "/$SYS"
    echo "::endgroup::"
    args+=(-Dsubrandr=enabled)
fi

meson setup build $common_args "${args[@]}"
meson compile -C build
./build/mpv.com -v --no-config
