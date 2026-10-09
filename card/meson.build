# The values of cards and the algorithms on them; doesn't need GTK, so it
# can also be used by the (GTK-free) rules of the games and their tests
cardvalue_lib = static_library('CardValue', files('Value.cpp'),
                               include_directories: top_inc,
                               dependencies: dependency('YGP', include_type: 'system'),
                               pic: true)
cardvalue_dep = declare_dependency(link_with: cardvalue_lib,
                                   include_directories: top_inc,
                                   dependencies: dependency('YGP', include_type: 'system'))

card_sources = files('Widget.cpp', 'Images.cpp', 'Pile.cpp', 'Set.cpp',
                     'Game.cpp', 'DeckSelect.cpp', 'PlayerDlg.cpp',
                     'ScoreDlg.cpp', 'PlayerConnDlg.cpp', 'Player.cpp',
                     'Human.cpp', 'RemotePlayer.cpp', 'ComputerPlayer.cpp',
                     'Window.cpp')

card_headers = files('Widget.h', 'Images.h', 'Pile.h', 'Set.h', 'Window.h',
                     'Game.h', 'Human.h', 'DeckSelect.h', 'PlayerDlg.h',
                     'ScoreDlg.h', 'PlayerConnDlg.h', 'Player.h',
                     'RemotePlayer.h', 'ComputerPlayer.h', 'Message.h', 'MessageLock.h',
                     'Value.h', 'Cards.h', 'Random.h')

# Named like libtool's "-release": libCard-<release>.so
libcard = shared_library('Card-' + release, card_sources,
                         include_directories: top_inc,
                         link_whole: cardvalue_lib,
                         dependencies: deps + [rsvg_dep],
                         install: true)

install_headers(card_headers, subdir: 'card')

card_dep = declare_dependency(link_with: libcard,
                              include_directories: top_inc,
                              dependencies: deps)
