# Unit tests of the (GTK-free) rules and simulations of complete games played
# by computer players. Run with "meson test" (or "meson test --suite unit" /
# "meson test --suite simulation").
#
# The simulations play the seeds 1 - 200 by default; set CARDCOL_SIM_GAMES to
# change the number of games and CARDCOL_SIM_FIRST_SEED for the first seed.
# Every failure reports the seed of the game; it can be replayed with
# CARDCOL_SIM_FIRST_SEED=<seed> CARDCOL_SIM_GAMES=1.
boost_test_dep = dependency('boost', modules: ['unit_test_framework'],
                            required: get_option('tests'), include_type: 'system')
if not boost_test_dep.found()
  subdir_done()
endif

test_deps = [rules_dep, boost_test_dep]
# -Dchecks defines CHECK, which clashes with Boost.Test
test_args = ['-DBOOST_TEST_DYN_LINK', '-UCHECK']

test('card', executable('test-card', files('card.cpp'),
                        cpp_args: test_args, dependencies: test_deps),
     suite: 'unit')

foreach game : enabled_games
  foreach kind : ['unit', 'simulation']
    source = (kind == 'unit') ? game + '.cpp' : game + '-sim.cpp'
    if fs.is_file(source)
      test(game + '-' + kind, executable('test-@0@-@1@'.format(game, kind), files(source),
                            cpp_args: test_args, dependencies: test_deps),
           suite: kind, timeout: 300)
    endif
  endforeach
endforeach
