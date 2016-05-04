/*
 * Compiler.h
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

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QString>

class Compiler {
public:
	Compiler ();
	~Compiler ();

	bool execute ();
	QString complete () const;

	void setMainFile (const QFile& file);
	void setOutputFile (const QFile& file);
	void setFlexPath (const QDir& path);
	void setSourcePath (const QDir& path);
	void setLibraryPath (const QDir& path);
	void setPlayerVersion (const float version);
	void setQuality (const int value);
	void setSWC (const bool swc);

	QProcess* getProcess ();
	QString getOutputName () const;

	static Compiler& get ();

private:
	QFile _main;
	QFile _output;
	QDir _flex;
	QDir _source;
	QDir _lib;
	QProcess* _process;
	QElapsedTimer _etimer;
	float _player;
	int _quality;
	bool _swc;
};

#define SWFCompiler Compiler::get()
