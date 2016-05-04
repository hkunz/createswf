/*
 * CoreApplication.cpp
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

#include "CoreApplication.h"
#include "common/Compiler.h"
#include "constants/CompileMode.h"
#include "parsers/DirectoryParser.h"
#include "ports/System.h"
#include "common/Logger.h"

#include <stdlib.h>

#include <QDir>
#include <QFileOpenEvent>
#include <QRegExp>

using namespace CreateSWF::Internal;

CoreApplication::CoreApplication (int &argc, char** argv) :
	QApplication(argc, argv), _compiler(), _flexHome(), _gui(false), _debug(false), _swc(false)
{
}

CoreApplication::~CoreApplication ()
{
}

void CoreApplication::setModeGUI (bool enabled)
{
	_gui = enabled;
	if (_gui) {
		QProcess* process = _compiler.getProcess();
connect	(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onProcessComplete(int, QProcess::ExitStatus)));
}
}

void CoreApplication::setFlexHome (QDir& flexHome)
{
	_flexHome = flexHome;
}

bool CoreApplication::compile (const QDir& dir, DefinitionParser::CompileArguments& c) const
{
	AbstractAssetsParser* parser = NULL;

	if (dir.exists(DEFINITION_NAME)) {
		DefinitionParser* p = new DefinitionParser();
		p->setTargetDir(dir);
		p->getCompileArguments(c);
		if (c.mode != CompileMode::COMPILE_DEFINITION) {
			delete p;
		} else {
			parser = p;
		}
	}

	if (!parser) {
		if (c.name.isEmpty())
			c.name = System.getCurWorkDir().filePath(dir.dirName());
		if (c.player < 0)
			c.player = 11.1;
		if (c.quality < 0)
			c.quality = 100;

		DirectoryParser* p = new DirectoryParser();
		p->setTargetDir(dir);
		p->setMovieclipPattern("^mc\\d+__");
		p->setSpritePattern("^sp\\d+__");
		p->setSuffixIgnorePattern("___");
		parser = p;
	}

	parser->setTempDir(System.getTempDir());
	parser->setUseVector(!(c.player < 11));
	parser->parse();

	delete parser;
	parser = NULL;

	QString output = c.name;
	const bool swc = _swc | c.swc;

	if (output.at(0) == '.') {
		warning("invalid output name \'" + output + "\'");
		output = "output";
	}

	if (!output.contains(Content::STR_DOT_SWF) && !output.contains(Content::STR_DOT_SWC)) {
		output = output + (swc ? Content::STR_DOT_SWC : Content::STR_DOT_SWF);
	}

	_compiler.setMainFile(System.getTempDir().filePath(Content::STR_MAIN + Content::STR_DOT_AS));
	_compiler.setOutputFile(output);
	_compiler.setFlexPath(_flexHome);
	_compiler.setSourcePath(System.getTempDir());
	_compiler.setLibraryPath(QDir(""));
	_compiler.setPlayerVersion(c.player);
	_compiler.setQuality(c.quality);
	_compiler.setSWC(swc);
	_compiler.execute();

	if (!_gui) {
		QProcess* process = _compiler.getProcess();
		process->waitForFinished();
		onProcessComplete(process->exitCode(), process->exitStatus());
	}

	return true;
}

void CoreApplication::onProcessComplete (int exitCode, QProcess::ExitStatus status) const
{
	QString msg;
	if (exitCode != EXIT_SUCCESS) {
		msg = "Errors occured during compilation!\n\nPlease check if the assets have valid names "
				"as their names represent the class name. Class names cannot have special "
				"symbols in them, only alpha-numeric characters and underscores\n\n";
		msg.append(_compiler.getProcess()->readAllStandardOutput());
		error("compilation failed");
		error(msg);
	} else {
		if (status == QProcess::CrashExit) {
			debug("process was aborted");
		} else {
			const QString stime = _compiler.complete();
			msg = "Successfully created => " + _compiler.getOutputName() + " => " + stime;
		}
	}
	if (_gui) {
		emit processComplete(exitCode, status, msg);
	}
	if (!_debug) {
		System.removeDir(System.getTempDir().path());
	}
}

void CoreApplication::terminateCompilation ()
{
	debug("terminate compilation");
	QProcess* process = _compiler.getProcess();
	process->terminate();
}

void CoreApplication::setDebug (bool debug)
{
	_debug = debug;
}

void CoreApplication::setSWC (bool swc)
{
	_swc = swc;
}

bool CoreApplication::event (QEvent* event)
{
	if (event->type() == QEvent::FileOpen) {
		QFileOpenEvent* fileOpenEvent = static_cast<QFileOpenEvent*> (event);
		emit fileOpenRequest(fileOpenEvent->file());
		return true;
	}
	return QApplication::event(event);
}
