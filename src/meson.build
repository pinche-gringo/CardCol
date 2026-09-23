cardcol_sources = files('GameTypes.cpp', 'Settings.cpp', 'CardSizes.cpp',
                        'CardColAppl.cpp', 'CardCol.cpp')

game_sources = {
  'buraco': files('Buraco.cpp', 'BuracoPile.cpp', 'BuracoCards.cpp'),
  'hearts': files('Hearts.cpp'),
  'jabberwocky': files('Jabberwocky.cpp'),
  'machiavelli': files('Machiavelli.cpp', 'MachiPile.cpp'),
  'rovhult': files('Rovhult.cpp', 'CardValue.cpp'),
  'sgtmayor': files('SgtMayor.cpp'),
  'twopart': files('Twopart.cpp'),
}
foreach game : enabled_games
  cardcol_sources += game_sources[game]
endforeach

if network
  cardcol_sources += files('ChatDlg.cpp', 'CardColNet.cpp')
endif

# Generate the code to read/write the options from their declaration
mgeni = find_program('mgeni.pl')
foreach meta : ['Options', 'CardOptions']
  cardcol_sources += custom_target(meta + '.meta',
                                   input: meta + '.h',
                                   output: meta + '.meta',
                                   command: [mgeni, '@INPUT@'],
                                   capture: true)
endforeach

executable('CardCol', cardcol_sources,
           dependencies: card_dep,
           cpp_args: ['-Weffc++'],
           install_rpath: prefix / get_option('libdir'),
           install: true)
