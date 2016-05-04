/*
 * MainWindow.cpp
 * Copyright (c) 2012, Harry Kunz <harry.kunz@ymail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. You must have great looks
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MainWindow.h"
#include "gui/AboutDialog.h"
#include "gui/PreferencesDialog.h"

#include <QMenu>
#include <QMenuBar>
#include <QApplication>

#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMimeData>
#include <QModelIndexList>
#include <QObject>
#include <QScrollBar>
#include <QSessionManager>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTextStream>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>
#include <QImageReader>
#include <QPushButton>
#include <QRegExp>
#include <QScrollBar>
#include <QSignalMapper>
#include <QShortcut>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolButton>
#include <QUrl>
#include <QWidget>

#include "common/Logger.h"

using namespace CreateSWF;
using namespace CreateSWF::Internal;

MainWindow::MainWindow (QWidget *parent, Qt::WindowFlags flags) :
	QMainWindow(parent, flags), _dirModel(NULL), _progressDialog(), _compileComplete(false)
{
	setupUi(this);
	init();
}

MainWindow::~MainWindow ()
{
	delete _dirModel;
}

void MainWindow::init ()
{
	ScrollBar* sbar = new ScrollBar(this);

	_toolbar = addToolBar("main toolbar");
	_vbLayout->setSpacing(1);
	_dirModel = new QStandardItemModel(0, 1, this);
	_dirList->setModel(_dirModel);
	_dirList->setVisible(_dirModel->rowCount() > 0);
	_dirList->resize(_dirList->width(), 0);
	_dirList->setFixedHeight(0);
	_dirList->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	_dirList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	_dirList->setVerticalScrollBar(sbar);
	_statusBar->showMessage("Ready");

	connect(_actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
	connect(_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(_actionOpen, SIGNAL(triggered()), this, SLOT(browseFile()));
	connect(_actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferencesDialog()));
	connect(_actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
	connect(_buttonAdd, SIGNAL(clicked()), this, SLOT(browseFile()));
	connect(_buttonRemove, SIGNAL(clicked()), this, SLOT(removeDirListItem()));
	connect(_buttonCompile, SIGNAL(clicked()), this, SLOT(compileTarget()));
	//connect(sbar, SIGNAL(onShow(bool)), this, SLOT(onHScrollBarShown(bool)));

	resize(width(), minimumHeight());
	setAcceptDrops(true);
}

void MainWindow::openAboutDialog () const
{
	AboutDialog dialog;
	dialog.exec();
}

void MainWindow::openPreferencesDialog () const
{
	PreferencesDialog dialog;
	dialog.exec();
}

void MainWindow::closeEvent (QCloseEvent *event)
{
}

void MainWindow::browseFile ()
{
	QList<QUrl> urls;
	urls << QUrl::fromLocalFile("/home/");
	QFileDialog dialog;
	dialog.setSidebarUrls(urls);
	dialog.setFileMode(QFileDialog::Directory);
	if (dialog.exec()) {
		addDirListItem(dialog.directory().path());
	}
}

void MainWindow::addDirListItem (QString path)
{
	_dirModel->setItem(_dirModel->rowCount(), 0, new QStandardItem(path));
	_dirList->setVisible(true);
	_dirList->setFocus(Qt::MouseFocusReason);
	_dirList->setCurrentIndex(_dirModel->index(_dirModel->rowCount() - 1, 0));
	_statusBar->showMessage("Ready");

	resizeDirList();
	resize(width(), minimumHeight());
}

void MainWindow::removeDirListItem ()
{
	int count = _dirModel->rowCount();
	if (count == 0)
		return;
	QModelIndexList indexes = _dirList->selectionModel()->selectedIndexes();
	for (int i = indexes.count() - 1; i > -1; --i) {
		_dirModel->removeRow(indexes.at(i).row());
	}
	count = _dirModel->rowCount();
	if (count == 0)
		_dirList->setVisible(false);
	resizeDirList();
	resize(width(), minimumHeight());
}

void MainWindow::compileTarget ()
{
	QDir target;
	int count = _dirModel->rowCount();
	if (count == 0) {
		const QString msg = "add a directory to the list";
		info(msg);
		_statusBar->showMessage(msg);
		return;
	}
	QModelIndexList indexes = _dirList->selectionModel()->selectedIndexes();
	int selections = 0;
	for (int i = indexes.count() - 1; i > -1; --i) {
		QStandardItem *item = _dirModel->item(indexes.at(i).row());
		target.setPath(item->text());
		++selections;
		break;
	}
	if (selections == 0) {
		const QString msg = "no compile target selected from list";
		info(msg);
		_statusBar->showMessage(msg);
		return;
	}
	target.makeAbsolute();
	if (!target.exists()) {
		const QString err = "directory " + target.path() + " does not exist";
		QMessageBox mbox;
		mbox.setText(err);
		mbox.exec();
		error(err);
		_statusBar->showMessage(err);
		return;
	}
	_compileComplete = false;
	_statusBar->showMessage("Compiling... please wait..");
	DefinitionParser::CompileArguments c;
	//TODO: need to be able to set mode & swc in the UI
	c.mode = CompileMode::UNDEFINED;
	c.swc = false;
	emit startCompile(target, c);
	_progressDialog.setWindowTitle("mxmlc");
	_progressDialog.setLabelText("Compilation in progress...");
	_progressDialog.setFixedSize(250, 110);
	_progressDialog.setMinimum(0);
	_progressDialog.setMaximum(0);
	_progressDialog.exec();

	if (!_compileComplete) {
		emit abortCompile ();
	}

	_statusBar->showMessage("Ready");
}

void MainWindow::onCompileProcessComplete (int exitCode, QProcess::ExitStatus status, QString& msg)
{
	_compileComplete = true;
	_progressDialog.close();

	if (!msg.isEmpty()) {
		QMessageBox mbox;
		mbox.setText(msg);
		mbox.exec();
	}
}

void MainWindow::resizeDirList (int items)
{
	const int rows = _dirModel->rowCount();
	if (rows == 0) {
		return;
	}
	const int itemH = _dirList->sizeHintForRow(0);
	const int count = items > 0 ? items : rows;
	const int height = itemH * (count > 5 ? 5 : count) + 15;
	_dirList->setFixedHeight(height);
	_dirList->resize(_dirList->width(), height);
}

void MainWindow::onHScrollBarShown (bool shown)
{
	//resizeDirList(_dirList->count() + 1);
	resize(width(), minimumHeight());
}

void MainWindow::changeEvent (QEvent *event)
{
	QMainWindow::changeEvent(event);
	switch (event->type()) {
	case QEvent::LanguageChange:
		break;
	default:
		break;
	}
}

void MainWindow::keyPressEvent (QKeyEvent *event)
{
}

void MainWindow::keyReleaseEvent (QKeyEvent *event)
{
}

void MainWindow::dragEnterEvent (QDragEnterEvent *e)
{
	e->acceptProposedAction();
}

void MainWindow::dragMoveEvent (QDragMoveEvent *e)
{
	e->accept();
}

void MainWindow::dropEvent (QDropEvent *e)
{
	const QMimeData* mimeData = e->mimeData();
	QList<QUrl> urls = mimeData->urls();
	for (QList<QUrl>::iterator iter = urls.begin(); iter != urls.end(); ++iter) {
		QString path = (*iter).path();
		QDir dir(path);
		if (dir.exists()) {
			addDirListItem(path);
		}
	}
}

void MainWindow::resizeEvent (QResizeEvent* event)
{
	//_container->resize(this->width(), 100);
	//_dirList->resize(this->width(), 50);
	//setFixedHeight(sizeHint().height());
}

