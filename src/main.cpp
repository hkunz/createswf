/*
 * main.cpp
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

#include <stdlib.h>
#include <QTextStream>
#include <QDesktopWidget>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QObject>

#include "CoreApplication.h"
#include "common/Logger.h"
#include "common/Version.h"
#include "gui/MainWindow.h"
#include "parsers/CommandLineParser.h"
#include "parsers/DefinitionParser.h"
#include "ports/System.h"

using namespace CreateSWF::Internal;

void center (QWidget &widget);

int main (int argc, char** argv)
{
	info("welcome to \'" + QLatin1String(APPFULLNAME) + "\' project: " + QLatin1String(argv[0]));

#ifdef Q_WS_X11
	QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif
	CoreApplication a(argc, argv);
	a.setOrganizationDomain(QLatin1String("createswf.org"));
	a.setApplicationName(QLatin1String(APPFULLNAME));
#ifdef BUILD_INFO_VERSION
	a.setApplicationVersion(QLatin1String(AS_STRING(BUILD_INFO_VERSION)));
#else
	a.setApplicationVersion(QLatin1String(APPVERSION));
#endif
#ifdef Q_WS_MAC
	a.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

	CommandLineParser cmd;
	bool uimode = true;
	const char* flexHome = getenv("FLEX_HOME");

	if (!flexHome || *flexHome == 0 || *flexHome == ' ') {
		QMessageBox msg;
		QString errmsg = "Error: Environment variable FLEX_HOME must point to the Flex SDK directory. ";
		errmsg += "You can obtain a free copy of the SDK at http://www.adobe.com/devnet/flex/flex-sdk-download.html";
		msg.setText(errmsg);
		msg.exec();
		error(errmsg);
		return EXIT_FAILURE;
	} else if (cmd.parse(argc, argv)) {
		return EXIT_FAILURE;
	} else if (cmd.getTargetDir() != NULL) {
		uimode = false;
	}

	QDir flexDir(flexHome);
	QDir dir(QString(cmd.getTargetDir()));

	if (!flexDir.exists("bin/mxmlc")) {
		QMessageBox msg;
		msg.setText("Error: Could not find mxmlc executable in " + flexDir.filePath("bin"));
		msg.exec();
		return EXIT_FAILURE;
	}

	a.setModeGUI(uimode);
	a.setFlexHome(flexDir);
	a.setDebug(cmd.isDebug());
	a.setSWC(cmd.isSWC());
	dir.makeAbsolute();

	if (!dir.exists()) {
		QMessageBox msg;
		QString errmsg = "Target directory \'" + dir.path() + "\' does not exist";
		msg.setText(errmsg);
		msg.exec();
		System.exit(errmsg, EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	if (uimode) {
		MainWindow w;
#ifdef __WIN32__
		QIcon appIcon(QLatin1String(":icon.icns"));
		w.setWindowIcon(appIcon);
#endif
		center(w);
		w.show();
		QObject::connect(&w, SIGNAL(startCompile(const QDir&, DefinitionParser::CompileArguments&)), &a, SLOT(compile(const QDir&, DefinitionParser::CompileArguments&)));
		QObject::connect(&w, SIGNAL(abortCompile()), &a, SLOT(terminateCompilation()));
		QObject::connect(&a, SIGNAL(processComplete(int, QProcess::ExitStatus, QString&)), &w, SLOT(onCompileProcessComplete(int, QProcess::ExitStatus, QString&)));
		return a.exec();
	}

	struct DefinitionParser::CompileArguments c;

	c.name = QString(cmd.getOutput() ? cmd.getOutput() : 0);
	c.mode = cmd.getCompileMode();
	c.player = cmd.getPlayer();
	c.quality = cmd.getQuality();
	c.swc = cmd.isSWC();

	a.compile(dir, c);

	return EXIT_SUCCESS;
}

void center (QWidget &widget)
{
	int x, y;
	int screenWidth;
	int screenHeight;
	int width, height;
	QSize windowSize;
	QDesktopWidget *desktop = QApplication::desktop();
	width = widget.frameGeometry().width();
	height = widget.frameGeometry().height();
	screenWidth = desktop->width();
	screenHeight = desktop->height();
	x = (screenWidth - width) / 2;
	y = (screenHeight - height) / 2;
	widget.move(x, y);
}
