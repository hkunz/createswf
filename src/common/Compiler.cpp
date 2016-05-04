/*
 * Compiler.cpp
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

#include <QFileInfo>
#include <QFileInfoList>

#include "Compiler.h"
#include "common/Logger.h"
#include "constants/Content.h"
#include "ports/System.h"

Compiler::Compiler () :
	_main(), _output(), _flex(), _source(), _lib(), _process(), _etimer(), _player(-1), _quality(-1), _swc(false)
{
	_process = new QProcess();
}

Compiler::~Compiler ()
{
	if (_process) {
		delete _process;
		_process = NULL;
	}
}

bool Compiler::execute ()
{
	float player = _player;

	QDir flexLib(_flex.filePath("frameworks/libs"));
	QDir bin(_flex.filePath("bin"));
	QDir playerDir(flexLib.filePath("player"));
	QDir usePlayerDir(playerDir.filePath(QString::number(player)));

	if (!usePlayerDir.exists()) {
		player = 9;
		warning("no flex player directory " + usePlayerDir.path());
		playerDir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QFileInfoList list = playerDir.entryInfoList();

		for (int i = 0; i < list.size(); ++i) {
			const QFileInfo& fileInfo = list.at(i);
			const float found = fileInfo.fileName().toFloat();
			if (found > player) {
				player = found;
			}
		}
		usePlayerDir.setPath(playerDir.filePath(QString::number(player)));
		warning("using default player directory " + usePlayerDir.path());
	}

	info("Compile " + _main.fileName() + " as " + _output.fileName() + " ...");

	const QString width = "1";
	const QString height = "1";
	const QString bgcolor = "0xffffff";

	QStringList compile;
	QString compiler;
	if (_swc) {
		compiler = bin.filePath("compc");
		compile.append("-source-path");
		compile.append(_source.path());
		compile.append("-include-classes");
		compile.append(Content::STR_MAIN);
	} else {
		compiler = bin.filePath("mxmlc");
		compile.append(_main.fileName());
		compile.append("-default-size=" + width + "," + height);
		compile.append("-default-background-color=" + bgcolor);
		compile.append("-static-link-runtime-shared-libraries=true");
	}

	compile.append("-output");
	compile.append(_output.fileName());
	compile.append("-target-player=" + QString::number(player));
	compile.append("-strict");
	compile.append("-library-path+=" + usePlayerDir.filePath("playerglobal.swc"));
	compile.append("-library-path+=" + flexLib.filePath("core.swc"));
	compile.append("-use-network=true");
	compile.append("-library-path+=" + _lib.path());
	compile.append("-define=CONFIG::DEBUG,true");
	compile.append("-define=CONFIG::FP10,true");
	compile.append("-define=CONFIG::FP9,false");

	_etimer.start();
	_process->setProcessChannelMode(QProcess::MergedChannels);
	_process->start(compiler, compile);

	return true;
}

QString Compiler::complete () const
{
	const float elapsed = _etimer.elapsed() / (float) 1000;
	float size = _output.size() / (float) (1024 * 1024);
	const int precision = size < 0.1 ? 10000 : 100;
	size = int(size * precision) / (float) precision;
	QString r = "Compilation of " + QString::number(size) + " MB completed in " + QString::number(elapsed) + " sec";
	info(r);
	return r;
}

void Compiler::setMainFile (const QFile& file)
{
	_main.setFileName(file.fileName());
}

void Compiler::setOutputFile (const QFile& file)
{
	_output.setFileName(file.fileName());
}

void Compiler::setSourcePath (const QDir& path)
{
	_source = path;
}

void Compiler::setLibraryPath (const QDir& path)
{
	_lib = path;
}

void Compiler::setFlexPath (const QDir& path)
{
	_flex = path;
}

void Compiler::setPlayerVersion (const float version)
{
	_player = version;
}

void Compiler::setQuality (const int value)
{
	_quality = value;
}

void Compiler::setSWC (const bool swc)
{
	_swc = swc;
}

QProcess* Compiler::getProcess ()
{
	return _process;
}

QString Compiler::getOutputName () const
{
	return _output.fileName();
}

Compiler& Compiler::get ()
{
	static Compiler _instance;
	return _instance;
}
