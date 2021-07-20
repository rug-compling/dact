resource_files = qt5.preprocess(
  qresources: ['dact.qrc']
)

if is_darwin
  info_conf_data = configuration_data()
  info_conf_data.set('dact_version', meson.project_version())
  info_plist = configure_file(input: 'Info.plist.in',
    output: 'Info.plist', configuration: info_conf_data)

  install_data(info_plist, install_dir: 'Contents')
  install_data(['dact-espresso.icns', 'dact-corpus.icns'],
    install_dir: 'Contents/Resources')

  if use_sandboxing
    install_data(['dact.entitlements'], install_dir: 'Contents/Resources')
  endif

  qmake = find_program('qmake-qt5','qmake')
  signing_key = get_option('signing_key')
  qt_dir = run_command(qmake, '-query', 'QT_HOST_PREFIX').stdout().strip('\n')
  meson.add_install_script('macos-bundler.sh', qt_dir, signing_key)
else
  install_data(
    'unix/dact.desktop',
    install_dir: get_option('datadir') / 'applications'
  )

  install_data(
    'dact-espresso.svg',
    rename: 'dact.svg',
    install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
  )
endif
