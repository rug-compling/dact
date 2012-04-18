#include <QDateTime>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QTextStream>
#include <QVector>

#include <stdexcept>
#include <typeinfo>

#include "BracketedDelegate.hh"
#include "BracketedColorDelegate.hh"
#include "BracketedKeywordInContextDelegate.hh"
#include "BracketedVisibilityDelegate.hh"
#include "BracketedWindow.hh"
#include "CorpusWidget.hh"
#include "DactMacrosModel.hh"
#include "FilterModel.hh"
#include "Query.hh"
#include "ValidityColor.hh"
#include "ui_BracketedWindow.h"

namespace ac = alpinocorpus;

BracketedWindow::BracketedWindow(QWidget *parent) :
    CorpusWidget(parent),
    d_ui(QSharedPointer<Ui::BracketedWindow>(new Ui::BracketedWindow))
{
    d_ui->setupUi(this);

    initListDelegates();
    createActions();
    readSettings();
}

BracketedWindow::~BracketedWindow()
{
    writeSettings();
}

void BracketedWindow::cancelQuery()
{
    if (d_model)
        d_model->cancelQuery();
}

void BracketedWindow::queryFailed(QString error)
{
    progressStopped(0, 0);

    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void BracketedWindow::switchCorpus(QSharedPointer<ac::CorpusReader> corpusReader)
{
    setReady(2, false);
    d_corpusReader = corpusReader;
}

void BracketedWindow::setFilter(QString const &filter, QString const &raw_filter)
{
    Q_UNUSED(raw_filter);

    setReady(2, false);
    d_filter = filter;
    startQuery();
}

void BracketedWindow::setModel(FilterModel *model)
{
    d_model = QSharedPointer<FilterModel>(model);
    d_ui->resultsTable->setModel(d_model.data());

    //d_ui->resultsTable->setColumnHidden(1, true);

    //d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);

    d_ui->resultsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d_ui->resultsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d_ui->resultsTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // disables horizontal jumping when a sentence is selected
    d_ui->resultsTable->setAutoScroll(false);

    /*
    connect(d_model.data(), SIGNAL(queryEntryFound(QString)),
        this, SLOT(updateResultsTotalCount()));
    */

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));

    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));

    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(queryFinished(int, int, bool)),
            SLOT(progressFinished(int, int, bool)));

}

void BracketedWindow::startQuery()
{
    // XXX - only once
    QFile file(":/stylesheets/bracketed-sentence.xsl");
    file.open(QIODevice::ReadOnly);
    QTextStream xslStream(&file);
    QString stylesheet = xslStream.readAll();

    if (d_filter.trimmed().isEmpty())
        setModel(new FilterModel(QSharedPointer<ac::CorpusReader>()));
    else
        setModel(new FilterModel(d_corpusReader));

    // Reload the list delegate since they keep their results cached.
    // This will make sure no old cached data is used.
    reloadListDelegate();

    d_model->runQuery(generateQuery(d_filter, "(@cat or @root)"),
        stylesheet);

    showFilenamesChanged();
}

void BracketedWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void BracketedWindow::createActions()
{
    /*
    QObject::connect(d_ui->resultsListWidget,
        SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
        this,
        SLOT(entrySelected(QListWidgetItem*,QListWidgetItem*)));
    */

    QObject::connect(d_ui->resultsTable,
        // itemActivated is triggered by a single click on some systems
        // where this is the configured behavior: it can be annoying.
        // But it also enables using [enter] to raise the main window
        // which is the expected/preferred behavior.
        SIGNAL(activated(QModelIndex const &)),
        this,
        SLOT(entryActivated(QModelIndex const &)));

    QObject::connect(d_ui->listDelegateComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(listDelegateChanged(int)));

    connect(d_ui->filenamesCheckBox, SIGNAL(toggled(bool)),
        SLOT(showFilenamesChanged()));
}

void BracketedWindow::showFilenames(bool show)
{
   d_ui->resultsTable->setColumnHidden(0, !show);
   d_ui->resultsTable->setColumnHidden(1, !show);
   d_ui->filenamesCheckBox->setChecked(show);
}

void BracketedWindow::showFilenamesChanged()
{
    showFilenames(d_ui->filenamesCheckBox->isChecked());
}

/*
void BracketedWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0)
        return;

    emit currentEntryChanged(current->data(Qt::UserRole).toString());

    // Raises this window again when using cursor keys after using
    // [enter] to raise the main window.
    raise();
}
*/


void BracketedWindow::entryActivated(QModelIndex const &index)
{
    emit entryActivated(index.sibling(index.row(), 0).data(Qt::UserRole).toString());
}

void BracketedWindow::addListDelegate(QString const &name, DelegateFactory factory)
{
    d_ui->listDelegateComboBox->addItem(name, d_listDelegateFactories.size());
    d_listDelegateFactories.append(factory);
}

void BracketedWindow::listDelegateChanged(int index)
{
    int delegateIndex = d_ui->listDelegateComboBox->itemData(index, Qt::UserRole).toInt();

    if (delegateIndex >= d_listDelegateFactories.size())
    {
        qWarning() << QString("Trying to select a list delegate (%1) beyond the boundary "
                              "of the d_listDelegateFactories list (%2)")
                             .arg(delegateIndex).arg(d_listDelegateFactories.size());
        return;
    }

    QAbstractItemDelegate* prevItemDelegate = d_ui->resultsTable->itemDelegateForColumn(2);
    d_ui->resultsTable->setItemDelegateForColumn(2, d_listDelegateFactories[delegateIndex](d_corpusReader));
    delete prevItemDelegate;
    d_ui->resultsTable->resizeRowsToContents();
}

void BracketedWindow::initListDelegates()
{
    addListDelegate("Complete sentence", &BracketedWindow::colorDelegateFactory);
    addListDelegate("Only matches", &BracketedWindow::visibilityDelegateFactory);
    addListDelegate("Keyword in Context", &BracketedWindow::keywordInContextDelegateFactory);
}

void BracketedWindow::reloadListDelegate()
{
    listDelegateChanged(d_ui->listDelegateComboBox->currentIndex());
}

void BracketedWindow::progressStarted(int totalEntries)
{
    setReady(2, false);
    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void BracketedWindow::progressChanged(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setValue(processedEntries);
}

void BracketedWindow::progressFinished(int processedEntries, int totalEntries, bool cached)
{
    progressStopped(processedEntries, totalEntries);
}


void BracketedWindow::progressStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
    if (d_model.isNull())
        setReady(2, false);
    else
        setReady(2, d_model->rowCount(QModelIndex()) ? true : false);
}

void BracketedWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void BracketedWindow::readSettings()
{
    QSettings settings;

    bool show = settings.value("bracketed_show_filenames", true).toBool();
    showFilenames(show);


    /*

    // restore last selected display method
    int delegateIndex = settings.value("filter_list_delegate", 0).toInt();
    listDelegateChanged(delegateIndex);
    d_ui->listDelegateComboBox->setCurrentIndex(delegateIndex);

    */

}

void BracketedWindow::writeSettings()
{
    QSettings settings;

    settings.setValue("bracketed_show_filenames", d_ui->filenamesCheckBox->isChecked());

    // display method
    settings.setValue("filter_list_delegate", d_ui->listDelegateComboBox->currentIndex());
}

void BracketedWindow::copy()
{
    QString output;
    QTextStream textstream(&output, QIODevice::WriteOnly | QIODevice::Text);

    selectionAsCSV(textstream);

    if (!output.isEmpty())
        QApplication::clipboard()->setText(output);
}

void BracketedWindow::exportSelection()
{
    QString filename(QFileDialog::getSaveFileName(this,
        "Export selection",
        QString(), "*.txt"));

    if (filename.isNull())
        return;

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,
            tr("Error exporting selection"),
            tr("Could open file for writing."),
            QMessageBox::Ok);

        return;
    }

    QTextStream textstream(&file);

    textstream.setGenerateByteOrderMark(true);
    selectionAsCSV(textstream);

    file.close();
}

void BracketedWindow::selectionAsCSV(QTextStream &output)
{
    // Nothing loaded? Do nothing.
    if (!d_model)
        return;

    QModelIndexList rows = d_ui->resultsTable->selectionModel()->selectedRows();

    // If there is nothing selected, do nothing
    if (rows.isEmpty())
        return;

    BracketedDelegate* delegate = dynamic_cast<BracketedDelegate*>(d_ui->resultsTable->itemDelegateForColumn(2));

    // Could not cast QAbstractItemDelegate to BracketedDelegate? Typical, but it is possible.
    if (!delegate)
        return;

    foreach (QModelIndex const &row, rows)
    {
        // This only works if the selection behavior is SelectRows
        output << delegate->sentenceForClipboard(row)
               << '\n';
    }

    output.flush();
}

QStyledItemDelegate* BracketedWindow::colorDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedColorDelegate(reader);
}

QStyledItemDelegate* BracketedWindow::visibilityDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedVisibilityDelegate(reader);
}

QStyledItemDelegate* BracketedWindow::keywordInContextDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedKeywordInContextDelegate(reader);
}

void BracketedWindow::saveAs()
{
    if (d_model.isNull())
        return;

    int nlines = d_model->rowCount(QModelIndex());

    if (nlines == 0)
        return;

    QString filename;
    QStringList filenames;

    QFileDialog::QFileDialog fd(this, tr("Save"), QString(), tr("Text (*.txt);;HTML (*.html *.htm)"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.setConfirmOverwrite(true);
    fd.setLabelText(QFileDialog::Accept, tr("Save"));
    if (d_lastfilterchoice.size())
        fd.selectNameFilter(d_lastfilterchoice);
    if (fd.exec())
        filenames = fd.selectedFiles();
    else
        return;
    if (filenames.size() < 1)
        return;
    filename = filenames[0];
    if (! filename.length())
        return;

    bool txt = false;
    bool html = false;
    d_lastfilterchoice = fd.selectedNameFilter();
    if (d_lastfilterchoice.contains("*.txt"))
        txt = true;
    else
        html = true;

    QFile data(filename);
    if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(this,
                              tr("Save file error"),
                              tr("Cannot save file %1 (error code %2)").arg(filename).arg(data.error()),
                              QMessageBox::Ok);
        return;
    }

    QString lbl;
    QString date(QDateTime::currentDateTime().toLocalTime().toString());
    qreal perc;
    int count;
    QTextStream out(&data);

    out.setCodec("UTF-8");

    if (txt)
        out << tr("Corpus") << ":\t" << d_corpusReader->name().c_str() << "\n"
            << tr("Filter") << ":\t" << d_filter << "\n"
            << tr("Date") << ":\t" << date << "\n"
            << "\n";

    if (html) {
        out <<
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
            "<html>\n"
            "  <head>\n"
            "    <title></title>\n"
            "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
            "<style type=\"text/css\">\n"
            "<!--\n"
            "body { background-color: #fff; color: #000; }\n"
            "body.show a.show,\n"
            "body.hide a.hide {\n"
            "  display: none;\n"
            "  visibility: hidden;\n"
            "}\n"
            "body.hide a.show,\n"
            "body.show a.hide {\n"
            "  padding: .4em .6em;\n"
            "  border: 1px solid #808080;\n"
            "  text-decoration: none;\n"
            "  color: black;\n"
            "}\n"
            "a.show:hover, a.hide:hover {\n"
            "  background-color: #c0c0FF;\n"
            "}\n"
            "body.hide dt,\n"
            "body.hide div.f,\n"
            "body.hide a.hide {\n"
            "  display: none;\n"
            "  visibility: hidden;\n"
            "}\n"
            "body.hide a.show {\n"
            "  display: inline;\n"
            "  visibility: visible;\n"
            "}\n"
            "body.hide dd {\n"
            "  margin-left: 0px;\n"
            "  margin-bottom: .5em;\n"
            "}\n"
            "body.show a.show {\n"
            "  display: none;\n"
            "  visibility: hidden;\n"
            "}\n"
            "body.show a.hide {\n"
            "  display: inline;\n"
            "  visibility: visible;\n"
            "}\n"
            "dd span { color: #FFF; }\n"
            ".l1 { background-color: #7FCDBB; }\n"
            ".l2 { background-color: #41B6C4; }\n"
            ".l3 { background-color: #1D91C0; }\n"
            ".l4 { background-color: #225EA8; }\n"
            ".l5 { background-color: #0C2C84; }\n"
            "b { color: #80e; }\n"
            "table { border-bottom: 1px solid #ccc; }\n"
            "-->\n"
            "</style>\n"
            "<!--[if !IE]> -->\n"
            "<style>\n"
            "td.l, td.r {\n"
            "  white-space: nowrap;\n"
            "  overflow: hidden;\n"
            "   max-width: 100px;\n"
            "}\n"
            "td.l {\n"
            "  direction: rtl;\n"
            "}\n"
            "</style>\n"
            "<!-- <![endif]-->\n"
            "<script language=\"JavaScript\"><!--\n"
            "function show() {\n"
            "  document.getElementById('main').className = 'show';\n"
            "}\n"
            "function hide() {\n"
            "  document.getElementById('main').className = 'hide';\n"
            "}\n"
            "//--></script>\n"
            "  </head>\n"
            "  <body id=\"main\" class=\"" << (d_ui->filenamesCheckBox->isChecked() ? "show" : "hide") << "\">\n"
            "    <table>\n"
            "      <tr><td>" << HTMLescape(tr("Corpus"))  << ":</td><td>" << HTMLescape(d_corpusReader->name()) << "</td></tr>\n"
            "      <tr><td>" << HTMLescape(tr("Filter"))  << ":</td><td>" << HTMLescape(d_filter) << "</td></tr>\n"
            "      <tr><td>" << HTMLescape(tr("Date"))  << ":</td><td>" << HTMLescape(date) << "</td></tr>\n"
            "    </table>\n"
            "    <p>\n"
            "    <a href=\"javascript:show()\" class=\"show\">" << tr("show filenames") << "</a>\n"
            "    <a href=\"javascript:hide()\" class=\"hide\">" << tr("hide filenames") << "</a>\n"
            "    <p>\n";

    }

    switch (d_ui->listDelegateComboBox->currentIndex()) {
    case 0:
        saveAsSentences(out, txt, html);
        break;
    case 1:
        saveAsMatches(out, txt, html);
        break;
    case 2:
        saveAsContext(out, txt, html);
        break;
    }

    if (html)
        out << "  </body>\n"
               "</html>\n";

    out.flush();
    data.close();

    emit statusMessage(tr("File saved as %1").arg(filename));

    /*
    QMessageBox::information(this,
                             tr("File saved"),
                             tr("File saved as %1").arg(filename),
                             QMessageBox::Ok);
    */
}

void BracketedWindow::saveAsSentences(QTextStream &out, bool txt, bool html)
{
    size_t nlines;

    bool filenames = d_ui->filenamesCheckBox->isChecked();

    if (txt) {
        nlines = d_model->rowCount(QModelIndex());
        for (size_t i = 0; i < nlines; i++) {
            if (filenames)
                out << d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString() << "\t"
                    << d_model->data(d_model->index(i, 1), Qt::DisplayRole).toInt() << "\t";
            out << d_model->data(d_model->index(i, 2), Qt::DisplayRole).toString().trimmed() << "\n";
        }
    }

    if (html) {

        out << "<dl>\n";

        nlines = d_model->rowCount(QModelIndex());
        for (size_t i = 0; i < nlines; i++) {
            out << "<dt>" << HTMLescape(d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString()) << " ["
                << d_model->data(d_model->index(i, 1), Qt::DisplayRole).toInt() << "]\n"
                << "<dd>";
            saveAsColorString(out, HTMLescape(d_model->data(d_model->index(i, 2), Qt::DisplayRole).toString()).trimmed());
            out << "\n";
        }

        out << "</dl>\n";

    }
}

void BracketedWindow::saveAsColorString(QTextStream &out, QString s)
{
    int level = 0;
    int lvl2 = 0;
    QString w;

    QStringList list = s.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    for (int i = 0; i < list.size(); ++i) {
        w = list.at(i);
        while (w.startsWith("[")) {
            level++;
            out << "<span class=\"l" << level << "\">";
            w = w.right(w.size() - 1);
        }
        lvl2 = 0;
        while (w.endsWith("]")) {
            lvl2++;
            w = w.left(w.size() - 1);
        }
        out << w;
        for (int j = 0; j < lvl2; j++)
            out << "</span>";
        out << " ";
        level -= lvl2;
    }

}

void BracketedWindow::saveAsMatches(QTextStream &out, bool txt, bool html)
{
    int i, i1, i2;
    int nlines = d_model->rowCount(QModelIndex());
    bool filenames = d_ui->filenamesCheckBox->isChecked();
    std::map<int, int> parts;
    std::map<int,int>::iterator it;
    QRegExp re("\\[[^\\[\\]]*\\]");

    if (html)
        out << "<dl>\n";

    for (i = 0; i < nlines; i++) {
        QString s = d_model->data(d_model->index(i, 2), Qt::DisplayRole).toString().trimmed();
        parts.clear();
        for (;;) {
            i1 = s.indexOf(re);
            if (i1 < 0)
                break;
            i2 = s.indexOf("]", i1);
            parts[i1] = i2;
            s[i1] = ' ';
            s[i2] = ' ';
        }

        if (txt) {
            if (filenames)
                out << d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString() << "\n";
            for (it = parts.begin(); it != parts.end(); it++) {
                if (filenames)
                    out << "\t";
                out << squeeze(s.mid((*it).first, (*it).second - (*it).first + 1)) << "\n";
            }
        }

        if (html) {
            out << "<dt>" << HTMLescape(d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString()) << "\n";
            for (it = parts.begin(); it != parts.end(); it++)
                out << "<dd>" << HTMLescape(s.mid((*it).first, (*it).second - (*it).first + 1)) << "\n";
        }

    }

    if (html)
        out << "</dl>\n";


}

void BracketedWindow::saveAsContext(QTextStream &out, bool txt, bool html)
{
    int i, i1, i2;
    int nlines = d_model->rowCount(QModelIndex());
    bool filenames = d_ui->filenamesCheckBox->isChecked();
    std::map<int, int> parts;
    std::map<int,int>::iterator it;
    QRegExp re("\\[[^\\[\\]]*\\]");
    QString prefix = filenames ? "\t" : "";

    for (i = 0; i < nlines; i++) {
        QString s = d_model->data(d_model->index(i, 2), Qt::DisplayRole).toString().trimmed();
        parts.clear();
        for (;;) {
            i1 = s.indexOf(re);
            if (i1 < 0)
                break;
            i2 = s.indexOf("]", i1);
            parts[i1] = i2;
            s[i1] = ' ';
            s[i2] = ' ';
        }

        if (txt) {
            if (filenames)
                out << d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString() << "\n";
            for (it = parts.begin(); it != parts.end(); it++) {
                out << prefix << squeeze(s.left((*it).first)) << "\n";
                /*
                out << prefix << "\t" << squeeze(s.mid((*it).first, (*it).second - (*it).first + 1).toUpper()) << " " <<
                    squeeze(s.mid((*it).second)) << "\n";
                */
                out << prefix << "\t" << squeeze(s.mid((*it).first, (*it).second - (*it).first + 1)) << "\n";
                out << prefix << "\t\t" << squeeze(s.mid((*it).second)) << "\n";
            }
        }

        if (html) {
            out << "<div class=\"f\">" << HTMLescape(d_model->data(d_model->index(i, 0), Qt::DisplayRole).toString()) << "</div>\n"
                << "<table width=\"100%\">\n";
            for (it = parts.begin(); it != parts.end(); it++)
                out << "<tr><td width=\"40%\" align=\"right\" valign=\"top\" class=\"l\">"
                    << HTMLescape(s.left((*it).first))
                    << "</td><td valign=\"bottom\" class=\"r\"><b>"
                    << HTMLescape(s.mid((*it).first, (*it).second - (*it).first + 1))
                    << "</b> "
                    << HTMLescape(s.mid((*it).second))
                    << "</td></tr>\n";
            out << "</table>\n";

        }
    }
}


QString BracketedWindow::HTMLescape(QString s)
{
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

QString BracketedWindow::HTMLescape(std::string s)
{
    return HTMLescape(QString(s.c_str()));
}

QString BracketedWindow::squeeze(QString const s)
{
    return s.trimmed().replace(QRegExp("\\s+"), " ");
}
