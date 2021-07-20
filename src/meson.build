extra_cpp_args = []

dact_sources = [
  'AboutWindow.cpp',
  'ArchiveModel.cpp',
  'ArchiveListItemDelegate.cpp',
  'ArchiveListItemWidget.cpp',
  'DactColorWell.cpp',
  'BracketedDelegates/BracketedDelegate.cpp',
  'BracketedDelegates/BracketedColorDelegate.cpp',
  'BracketedDelegates/BracketedVisibilityDelegate.cpp',
  'BracketedDelegates/BracketedKeywordInContextDelegate.cpp',
  'BracketedSentenceWidget.cpp',
  'BracketedWindow.cpp',
  'DependencyTreeWidget.cpp',
  'DactApplication.cpp',
  'DactApplicationEvent.cpp',
  'HistoryComboBox.cpp',
  'DactListView.cpp',
  'DactMacrosFile.cpp',
  'DactMacrosMenu.cpp',
  'DactMacrosModel.cpp',
  'DactSettings.cpp',
  'DactTableView.cpp',
  'DactTool.cpp',
  'DactToolsMenu.cpp',
  'DactToolsModel.cpp',
  'DelayedLoadFileCallback.cpp',
  'DactMenuBar.cpp',
  'Edge.cpp',
  'QueryModel.cpp',
  'QueryScope.cpp',
  'QtIOCompressor.cpp',
  'SimpleDTD.cpp',
  'SimpleDTDAttribute.cpp',
  'DactTreeScene.cpp',
  'DactTreeView.cpp',
  'FilterModel.cpp',
  'GlobalCopyCommand.cpp',
  'GlobalCutCommand.cpp',
  'GlobalEditCommand.cpp',
  'GlobalPasteCommand.cpp',
  'HumanReadableSize.cpp',
  'MainWindow.cpp',
  'OpenCorpusDialog.cpp',
  'PercentageCellDelegate.cpp',
  'PopupItem.cpp',
  'PreferencesWindow.cpp',
  'ProgramOptions.cpp',
  'QtLocalPeer.cpp',
  'QtSingleApplication.cpp',
  'RecentFilesMenu.cpp',
  'SecEdge.cpp',
  'StatisticsWindow.cpp',
  'TreeNode.cpp',
  'TreeNodeInspector.cpp',
  'XPathValidator.cpp',
  'XSLTransformer.cpp',
  'generateQuery.cpp',
  'main.cpp',
  'validityColor.cpp',
]

dact_deps = [
  alpinocorpus_dep,
  config_hh,
  libexslt_dep,
  libxml_dep,
  libxslt_dep,
  qt5_dep,
  xercesc_dep,
  xqilla_dep,
  zlib_dep
]

if is_darwin
  dact_sources += [
    'AppleUtils.mm'
  ]
  dact_deps += [
    appkit
  ]

  if use_sandboxing
    extra_cpp_args += [
      '-DUSE_SANDBOXING'
    ]
  endif
endif

executable('dact',
  dact_sources,
  moc_files,
  resource_files,
  ui_files,
  include_directories: inc,
  install: true,
  dependencies: dact_deps,
  cpp_args: extra_cpp_args,
)
