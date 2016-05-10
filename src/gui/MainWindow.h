/*
 * MainWindow.h
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

#pragma once

#include "ui_mainwindow.h"
#include "parsers/DefinitionParser.h"

#include <QDir>
#include <QMainWindow>
#include <QProcess>
#include <QProgressDialog>
#include <QScrollBar>

class QStandardItemModel;

namespace CreateSWF {
namespace Internal {

class MainWindow: public QMainWindow, private Ui::MainWindow {
	Q_OBJECT
public:
	MainWindow (QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~MainWindow ();

public slots:
	void onCompileProcessComplete (int exitCode, QProcess::ExitStatus status, QString&);

private:
	QStandardItemModel* _dirModel;
	QProgressDialog _progressDialog;
	bool _compileComplete;

	void init ();
	void resizeDirList (int items = 0);

private slots:
	void browseFile ();
	void openAboutDialog () const;
	void openPreferencesDialog () const;
	void addDirListItem (QString path);
	void removeDirListItem ();
	void compileTarget ();
	void onHScrollBarShown (bool shown);

protected:
	void closeEvent (QCloseEvent* event);
	void changeEvent (QEvent* event);
	void keyPressEvent (QKeyEvent* event);
	void keyReleaseEvent (QKeyEvent* event);
	void dragEnterEvent (QDragEnterEvent* event);
	void dropEvent (QDropEvent* event);
	void dragMoveEvent (QDragMoveEvent* event);
	void resizeEvent (QResizeEvent* event);

signals:
	void startCompile(const QDir& dir, DefinitionParser::CompileArguments& c) const;
	void abortCompile ();
};

class ScrollBar: public QScrollBar {
	Q_OBJECT
public:
	ScrollBar (QWidget *parent = 0) :
		QScrollBar(parent)
	{
	}
	~ScrollBar ()
	{
	}
	void hideEvent (QHideEvent *e) {
		Q_UNUSED(e);
		emit onShow(false);
	}
	void showEvent (QShowEvent *e) {
		Q_UNUSED(e);
		emit onShow(true);
	}

signals:
	void onShow (bool shown = true);
};

} // namespace Internal
} // namespace CreateSWF
