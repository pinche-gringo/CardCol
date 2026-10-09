cardcol_sources = files('GameTypes.cpp', 'Settings.cpp', 'CardSizes.cpp',
                        'CardColAppl.cpp', 'CardCol.cpp')

# The rules of the games (incl. the computer players); they don't depend on
# GTK, so they are also used by the tests
rules_sources = {
  'buraco': ['BuracoRules.cpp'],
  'hearts': ['HeartsRules.cpp'],
  'jabberwocky': ['JabberwockyRules.cpp'],
  'machiavelli': ['MachiavelliRules.cpp'],
  'rovhult': ['RovhultRules.cpp'],
  'sgtmayor': ['SgtMayorRules.cpp'],
  'twopart': ['TwopartRules.cpp'],
}
enabled_rules = []
foreach game : enabled_games
  enabled_rules += files(rules_sources[game])
endforeach
rules_lib = static_library('CardColRules', enabled_rules,
                           dependencies: cardvalue_dep)
rules_dep = declare_dependency(link_with: rules_lib,
                               include_directories: include_directories('.'),
                               dependencies: cardvalue_dep)

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
           dependencies: [card_dep, rules_dep],
           install_rpath: prefix / get_option('libdir'),
           install: true)
