card_sources = files('Widget.cpp', 'Images.cpp', 'Pile.cpp', 'Set.cpp',
                     'Game.cpp', 'DeckSelect.cpp', 'PlayerDlg.cpp',
                     'ScoreDlg.cpp', 'PlayerConnDlg.cpp', 'Player.cpp',
                     'Human.cpp', 'RemotePlayer.cpp', 'ComputerPlayer.cpp',
                     'Window.cpp')

card_headers = files('Widget.h', 'Images.h', 'Pile.h', 'Set.h', 'Window.h',
                     'Game.h', 'Human.h', 'DeckSelect.h', 'PlayerDlg.h',
                     'ScoreDlg.h', 'PlayerConnDlg.h', 'Player.h',
                     'RemotePlayer.h', 'ComputerPlayer.h', 'Message.h', 'Random.h')

# Named like libtool's "-release": libCard-<release>.so
libcard = shared_library('Card-' + release, card_sources,
                         include_directories: top_inc,
                         dependencies: deps + [rsvg_dep],
                         install: true)

install_headers(card_headers, subdir: 'card')

card_dep = declare_dependency(link_with: libcard,
                              include_directories: top_inc,
                              dependencies: deps)
