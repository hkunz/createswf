/*
 * CoreApplication.h
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

#include "common/Compiler.h"
#include "parsers/DefinitionParser.h"

#include <QApplication>
#include <QDir>
#include <QProcess>

namespace CreateSWF {
namespace Internal {

class CoreApplication: public QApplication {
	Q_OBJECT

public:
	CoreApplication (int &argc, char** argv);
	~CoreApplication ();

	void setModeGUI (bool enabled);
	void setFlexHome (QDir& flexHome);
	void setSWC (bool swc);
	void setDebug (bool debug);

public slots:
	void terminateCompilation ();

public slots:
	bool compile (const QDir& dir, DefinitionParser::CompileArguments& c) const;
	void onProcessComplete (int exitCode, QProcess::ExitStatus status) const;

private:
	mutable Compiler _compiler;
	QDir _flexHome;
	bool _gui;
	bool _debug;
	bool _swc;

	bool event (QEvent *);

signals:
	void processComplete(int exitCode, QProcess::ExitStatus status, QString&) const;
	void fileOpenRequest (const QString &file);
};

} // namespace Internal
} // namespace CreateSWF
