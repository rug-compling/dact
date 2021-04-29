config_hh = declare_dependency(
  sources: vcs_tag(
    command: ['git', 'log', '--pretty=format:%ad (commit: %h)', '--date=iso', '-n 1'],
    input: 'config.hh.in',
    output: 'config.hh'
  )
)

inc = include_directories('.')

moc_files = qt5.preprocess(
  moc_headers: [
    'AboutWindow.hh',
    'ArchiveModel.hh',
    'ArchiveListItemDelegate.hh',
    'ArchiveListItemWidget.hh',
    'DactApplication.hh',
    'DactColorWell.hh',
    'BracketedDelegate.hh',
    'BracketedColorDelegate.hh',
    'BracketedKeywordInContextDelegate.hh',
    'BracketedSentenceWidget.hh',
    'BracketedVisibilityDelegate.hh',
    'BracketedWindow.hh',
    'CorpusWidget.hh',
    'DependencyTreeWidget.hh',
    'DactListView.hh',
    'DactMacrosFile.hh',
    'DactMacrosMenu.hh',
    'DactMacrosModel.hh',
    'DactSettings.hh',
    'DactTableView.hh',
    'DactTool.hh',
    'DactToolsModel.hh',
    'DactTreeScene.hh',
    'DactTreeView.hh',
    'DelayedLoadFileCallback.hh',
    'DactMenuBar.hh',
    'HistoryComboBox.hh',
    'QtIOCompressor.hh',
    'QueryModel.hh',
    'FilterModel.hh',
    'GlobalCopyCommand.hh',
    'GlobalCutCommand.hh',
    'GlobalEditCommand.hh',
    'GlobalPasteCommand.hh',
    'MainWindow.hh',
    'OpenCorpusDialog.hh',
    'PercentageCellDelegate.hh',
    'PreferencesWindow.hh',
    'QtLocalPeer.hh',
    'QtSingleApplication.hh',
    'RecentFilesMenu.hh',
    'StatisticsWindow.hh',
    'TreeNodeInspector.hh',
    'XPathValidator.hh',
  ],
  include_directories: inc,
  dependencies: [
    config_hh,
    qt5_dep
  ])
