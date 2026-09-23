#!/bin/sh
# Prints the directory holding carddecks of the passed kind (kde, gnome or
# cardpics); if none of the usual locations exist, a default is printed.
# The result always ends with a slash. Validating the directory is done by
# meson.build.

case $1 in
   kde)
      # Instead of a fixed (and ever-aging) list of version numbers, discover
      # every <major>.<minor> version actually installed (by looking at the
      # version-specific directories distributions use) and probe them from
      # newest to oldest.
      kde_versions=
      for cardsdir in /opt/kde*/share/apps/carddecks \
                      /usr/share/kde*/share/apps/carddecks \
                      /usr/share/kde*/apps/carddecks \
                      /usr/kde/*/share/apps/carddecks \
                      /usr/kde*/share/apps/carddecks; do
         test -d "$cardsdir" || continue
         case $cardsdir in
            /opt/kde*)       v=${cardsdir#/opt/kde} ;;
            /usr/share/kde*) v=${cardsdir#/usr/share/kde} ;;
            /usr/kde/*)      v=${cardsdir#/usr/kde/} ;;
            /usr/kde*)       v=${cardsdir#/usr/kde} ;;
         esac
         v=${v%%/*}

         # keep only genuine <major>.<minor>, i.e. two dot-separated integers
         # - discard anything else (a bare "4", "4.2.1", "4.2-beta", ...)
         case $v in
            *.*.*) continue ;;
            *.*) ;;
            *) continue ;;
         esac
         kde_major=${v%%.*}
         kde_minor=${v#*.}
         case $kde_major in
            ''|*[!0-9]*) continue ;;
         esac
         case $kde_minor in
            ''|*[!0-9]*) continue ;;
         esac

         case " $kde_versions " in
            *" $v "*) ;;
            *) kde_versions="$kde_versions $v" ;;
         esac
      done

      # Sort descending, numerically on major and minor (not lexically - so
      # e.g. 4.10 sorts above 4.9)
      kde_versions=`for v in $kde_versions; do echo "$v"; done | sort -t. -k1,1nr -k2,2nr`

      for version in $kde_versions; do
         for dir in /opt/kde$version/share/apps/carddecks \
                    /usr/share/kde$version/share/apps/carddecks \
                    /usr/share/kde$version/apps/carddecks \
                    /usr/kde$version/share/apps/carddecks \
                    /usr/share/apps/carddecks \
                    /usr/local/share/apps/carddecks \
                    /usr/kde/$version/share/apps/carddecks; do
            if test -d "$dir/"; then
               echo "$dir/"
               exit 0
            fi
         done
      done
      echo /opt/kde3/share/apps/carddecks/
      ;;

   gnome)
      for dir in /usr/share/pixmaps/gnome-games-common/cards \
                 /usr/share/gnome/pixmaps/gnome-games-common/cards \
                 /usr/share/gnome/gnome-games-common/cards \
                 /usr/share/gnome/cards /usr/share/pixmaps/gnome/cards \
                 /usr/share/gnome/pixmaps/cards \
                 /usr/share/gnome-games-common/cards \
                 /usr/share/aisleriot/cards \
                 /usr/local/share/gnome-games-common/cards; do
         if test -d "$dir/"; then
            echo "$dir/"
            exit 0
         fi
      done
      echo /usr/share/pixmaps/gnome-games-common/cards/
      ;;

   cardpics)
      for dir in /usr/share/games/cardpics/images /usr/share/cardpics/images \
                 /usr/local/share/games/cardpics/images /usr/local/share/cardpics/images; do
         if test -d "$dir/"; then
            echo "$dir/"
            exit 0
         fi
      done
      echo /usr/share/games/cardpics/images/
      ;;

   *)
      echo "Usage: $0 kde|gnome|cardpics" >&2
      exit 1
      ;;
esac
