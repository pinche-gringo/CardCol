install_data('CardCol.desktop', install_dir: datadir / 'applications')

# Application icon, installed into the hicolor icon theme so desktop
# environments (file managers, task switchers, docks, ...) can find it by the
# name referenced in CardCol.desktop's Icon= entry. Each size is generated
# from the master image CardCol.png with ImageMagick.
convert = find_program('magick', 'convert', required: false)
if convert.found()
  icon_args = []
  icons = []
  foreach size : ['16x16', '22x22', '24x24', '32x32', '48x48', '64x64',
                  '128x128', '256x256']
    icon = custom_target('icon-' + size,
                         input: 'CardCol.png',
                         output: 'CardCol-@0@.png'.format(size),
                         command: [convert, '@INPUT@', '-filter', 'Lanczos',
                                   '-resize', size, '-strip', '@OUTPUT@'],
                         build_by_default: true)
    icons += icon
    icon_args += '@0@:@1@'.format(size, icon.full_path())
  endforeach

  meson.add_install_script(find_program('../build-aux/install-icons.sh'),
                           datadir, icon_args)

  # Let the desktop environment pick up the installed icon immediately
  gnome.post_install(gtk_update_icon_cache: true)
else
  warning('ImageMagick (magick or convert) not found - no icons are installed')
endif
