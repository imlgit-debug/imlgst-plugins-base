foreach plugin : ['adder', 'app', 'audioconvert', 'audiomixer', 'audiorate', 'audioresample',
                  'audiotestsrc', 'compositor', 'encoding', 'gio', 'imlscale', 'overlaycomposition', 'pbtypes', 'playback',
                  'rawparse', 'subparse', 'tcp', 'typefind', 'videoconvert', 'videorate', 'videoscale',
                  'videotestsrc', 'volume']
  if not get_option(plugin).disabled()
    subdir(plugin)
  endif
endforeach
