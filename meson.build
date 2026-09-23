project('CardCol', 'cpp',
        version: '0.9.52',
        license: 'GPL-2.0-or-later',
        meson_version: '>= 1.5.0',
        default_options: ['cpp_std=c++26', 'warning_level=2',
                          'buildtype=debugoptimized'])

fs = import('fs')
gnome = import('gnome')

package_name = 'Cardgames'
bugreport = 'g17m0@users.sourceforge.net'
version = meson.project_version()
version_parts = version.split('.')
release = '@0@.@1@'.format(version_parts[0], version_parts[1])

# Version of the communication protocoll
protocoll = 1
prot_variant = 3

cxx = meson.get_compiler('cpp')

prefix = get_option('prefix')
datadir = get_option('datadir')
docudir = datadir / 'doc' / meson.project_name()
localedir = get_option('localedir')

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_NAME', package_name)
conf.set_quoted('PACKAGE_TARNAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', version)
conf.set_quoted('PACKAGE_STRING', package_name + ' ' + version)
conf.set_quoted('PACKAGE_BUGREPORT', bugreport)
conf.set_quoted('VERSION', version)
conf.set_quoted('PRG_RELEASE', release)
conf.set('PROTOCOLL', protocoll)
conf.set('PROT_VARIANT', prot_variant)
conf.set_quoted('STRPROTOCOLL', protocoll.to_string())
conf.set_quoted('STRVARIANT', prot_variant.to_string())
conf.set_quoted('LOCALEDIR', prefix / localedir)
conf.set_quoted('DOCUDIR', prefix / docudir + '/')
conf.set('WORDS_BIGENDIAN', host_machine.endian() == 'big')

add_project_arguments('-D_GNU_SOURCE', '-Woverloaded-virtual', '-Wsuggest-override',
                      language: 'cpp')

debug_level = get_option('debug-level')
if debug_level >= 0
  add_project_arguments('-g', '-DTRACELEVEL=@0@'.format(debug_level), language: 'cpp')
endif
check_level = get_option('checks')
if check_level >= 0
  add_project_arguments('-DCHECK=@0@'.format(check_level), language: 'cpp')
endif
if get_option('saving')
  add_project_arguments('-DSAVE_GAME', language: 'cpp')
endif

# Dependencies
deps = [dependency('gtkmm-4.0', version: '>= 4.0.0', include_type: 'system'),
        dependency('gthread-2.0', version: '> 2.2.0', include_type: 'system'),
        dependency('YGP', version: '>= 0.9.30', include_type: 'system'),
        dependency('XGP', version: '>= 0.9.50', include_type: 'system')]

rsvg_dep = dependency('librsvg-2.0', required: get_option('rsvg'))
conf.set('HAVE_RSVG', rsvg_dep.found())

intl_dep = dependency('intl', required: get_option('nls'))
nls = intl_dep.found() and find_program('msgfmt', required: get_option('nls')).found()
conf.set('ENABLE_NLS', nls)
conf.set('HAVE_GETTEXT', nls)
if nls
  deps += intl_dep
endif

if get_option('network')
  thread_dep = dependency('threads', required: false)
  if thread_dep.found()
    deps += thread_dep
    conf.set('WITH_NETWORK', true)
  else
    warning('libpthread is not installed! Disabling network features')
  endif
endif
network = conf.get('WITH_NETWORK', false)

# Cardgames to include
games = ['rovhult', 'twopart', 'hearts', 'buraco', 'machiavelli', 'sgtmayor',
         'jabberwocky']
enabled_games = []
foreach game : games
  conf.set('WITH_' + game.to_upper(), get_option(game))
  if get_option(game)
    enabled_games += game
  endif
endforeach
if enabled_games.length() == 0
  error('All games are disabled!')
endif

# Carddecks
find_carddir = find_program('build-aux/find-carddir.sh')
have_cards = false

kde_dir = get_option('kde-carddir')
if kde_dir != 'no'
  if kde_dir == ''
    kde_dir = run_command(find_carddir, 'kde', check: true).stdout().strip()
  elif not kde_dir.endswith('/')
    kde_dir += '/'
  endif
  message('KDE carddecks: ' + kde_dir)
  if not fs.is_dir(kde_dir)
    warning('Directory `@0@\' does not exist!'.format(kde_dir))
  elif fs.is_dir(kde_dir / 'decks')
    have_cards = true
    conf.set_quoted('KDECARDS_DIR', kde_dir)
  else
    warning('Directory `@0@\' does not seem to hold KDE carddecks!'.format(kde_dir))
  endif
endif

gnome_dir = get_option('gnome-carddir')
if gnome_dir != 'no'
  if gnome_dir == ''
    gnome_dir = run_command(find_carddir, 'gnome', check: true).stdout().strip()
  elif not gnome_dir.endswith('/')
    gnome_dir += '/'
  endif
  message('GNOME carddecks: ' + gnome_dir)
  if not fs.is_dir(gnome_dir)
    warning('Directory `@0@\' does not exist!'.format(gnome_dir))
  else
    found = false
    foreach deck : ['bonded.png', 'bonded.svg', 'bonded.svgz', 'anglo.svgz',
                    'gnomangelo_bitmap.svg', 'paris.svg']
      found = found or fs.is_file(gnome_dir / deck)
    endforeach
    if found
      have_cards = true
      conf.set_quoted('GNOMECARDS_DIR', gnome_dir)
    else
      warning('Directory `@0@\' does not seem to hold GNOME carddecks!'.format(gnome_dir))
    endif
  endif
endif

cardpics_dir = get_option('cardpics-dir')
if cardpics_dir != 'no'
  if cardpics_dir == ''
    cardpics_dir = run_command(find_carddir, 'cardpics', check: true).stdout().strip()
  elif not cardpics_dir.endswith('/')
    cardpics_dir += '/'
  endif
  message('Cardpics: ' + cardpics_dir)
  if not fs.is_dir(cardpics_dir)
    warning('Directory `@0@\' does not exist!'.format(cardpics_dir))
  elif fs.is_file(cardpics_dir / '00.png')
    have_cards = true
    conf.set_quoted('CARDPICS_DIR', cardpics_dir)
  else
    warning('Directory `@0@\' does not seem to hold cardpics!'.format(cardpics_dir))
  endif
endif

if not have_cards
  error('Didn\'t find any carddecks!')
endif

configure_file(input: 'cardgames-cfg.h.meson',
               output: 'cardgames-cfg.h',
               configuration: conf)
top_inc = include_directories('.')

subdir('card')
subdir('src')
if nls
  subdir('po')
endif
subdir('doc')
subdir('data')

summary({'Network': network,
         'librsvg': rsvg_dep.found(),
         'NLS': nls,
         'Saving games': get_option('saving')}, section: 'Features')
summary({'Games': enabled_games}, section: 'Features')
