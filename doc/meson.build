docu = []
foreach game : ['CardCol', 'Rovhult', 'Twopart', 'Hearts', 'Buraco',
                'Machiavelli', 'Sgt.Mayor', 'Jabberwocky']
  foreach lang : ['de', 'en', 'es']
    docu += '@0@.html.@1@'.format(game, lang)
  endforeach
endforeach

install_data(docu, install_dir: docudir)
