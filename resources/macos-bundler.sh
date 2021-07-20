#!/bin/sh -eux

if [ $# -ne 2 ]; then
  >&2 echo "Usage: $0 QT_DIR SIGNING_KEY"
  exit 1
fi

QT_PATH="$1"
SIGNING_KEY="$2"

"${QT_PATH}/bin/macdeployqt" ${MESON_INSTALL_PREFIX} \
  -codesign="${SIGNING_KEY}"
  #-hardened-runtime
