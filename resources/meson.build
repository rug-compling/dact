resource_files = qt5.preprocess(
  qresources: ['dact.qrc']
)

install_data(
  'unix/dact.desktop',
  install_dir: get_option('datadir') / 'applications'
)

install_data(
  'dact-espresso.svg',
  rename: 'dact.svg',
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
)