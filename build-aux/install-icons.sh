#!/bin/sh
# Installs the generated icons (passed as <size>:<file>) as
# <datadir>/icons/hicolor/<size>/apps/CardCol.png.
# Called by meson install; $1 is the datadir relative to the prefix.

datadir=$1
shift
for icon; do
   size=${icon%%:*}
   dir="$MESON_INSTALL_DESTDIR_PREFIX/$datadir/icons/hicolor/$size/apps"
   echo "Installing $dir/CardCol.png"
   install -d "$dir" && install -m 644 "${icon#*:}" "$dir/CardCol.png" || exit 1
done
