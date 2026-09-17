SUBDIRS = .

desktopdir=$(datadir)/applications
desktop_DATA = CardCol.desktop

# Application icon, installed into the hicolor icon theme so desktop
# environments (file managers, task switchers, docks, ...) can find it
# by the name referenced in CardCol.desktop's Icon= entry.
#
# Each size below is generated from the master image CardCol.png by the
# pattern rule further down. They are also committed/shipped pre-generated,
# so building or installing this package never requires ImageMagick -- it
# is only needed again when CardCol.png itself is edited and the derived
# icons need to be refreshed (plain `make` picks that up automatically).
ICON_SIZES = 16x16 22x22 24x24 32x32 48x48 64x64 128x128 256x256
ICON_PNGS = $(ICON_SIZES:%=icons/hicolor/%/apps/CardCol.png)

icon16dir = $(datadir)/icons/hicolor/16x16/apps
icon16_DATA = icons/hicolor/16x16/apps/CardCol.png

icon22dir = $(datadir)/icons/hicolor/22x22/apps
icon22_DATA = icons/hicolor/22x22/apps/CardCol.png

icon24dir = $(datadir)/icons/hicolor/24x24/apps
icon24_DATA = icons/hicolor/24x24/apps/CardCol.png

icon32dir = $(datadir)/icons/hicolor/32x32/apps
icon32_DATA = icons/hicolor/32x32/apps/CardCol.png

icon48dir = $(datadir)/icons/hicolor/48x48/apps
icon48_DATA = icons/hicolor/48x48/apps/CardCol.png

icon64dir = $(datadir)/icons/hicolor/64x64/apps
icon64_DATA = icons/hicolor/64x64/apps/CardCol.png

icon128dir = $(datadir)/icons/hicolor/128x128/apps
icon128_DATA = icons/hicolor/128x128/apps/CardCol.png

icon256dir = $(datadir)/icons/hicolor/256x256/apps
icon256_DATA = icons/hicolor/256x256/apps/CardCol.png

EXTRA_DIST = $(desktop_DATA) CardCol.png $(ICON_PNGS)

# Make sure the icons are up to date whenever a plain build is done, not
# just when they are (re-)installed.
all-local: $(ICON_PNGS)

# Regenerate one installed icon size from the master image. Re-run whenever
# CardCol.png is newer than the size in question.
icons/hicolor/%/apps/CardCol.png: CardCol.png
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_GEN)if test -n "$(CONVERT)"; then \
	  $(CONVERT) "$(srcdir)/CardCol.png" -filter Lanczos -resize $* -strip "$@"; \
	else \
	  echo "*** ImageMagick (magick/convert) not found -- leaving $@ as-is." >&2; \
	  echo "*** Install ImageMagick and re-run configure to regenerate it from CardCol.png." >&2; \
	  touch "$@"; \
	fi

# Let the desktop environment pick up the (un)installed icon immediately,
# instead of waiting for it to notice the icon theme changed on its own.
install-data-hook:
	-gtk-update-icon-cache -f -t $(datadir)/icons/hicolor 2>/dev/null

uninstall-hook:
	-gtk-update-icon-cache -f -t $(datadir)/icons/hicolor 2>/dev/null
