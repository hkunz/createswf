/*
 * ISystem.h
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
#include <QString>

#include <stdlib.h>

#include "common/Logger.h"
#include "common/Version.h"

class ISystem {
private:
	ISystem (const ISystem&);
	ISystem& operator= (const ISystem&);

public:
	ISystem ()
	{
	}
	virtual ~ISystem ()
	{
	}

	virtual QDir getCurWorkDir () const = 0;
	virtual QString getCurrentUser () const = 0;

	virtual QDir getHomeDir () const
	{
		QDir home = QDir::home();
		static QString dir = QString(home.path()) + "/." + APPNAME + "/";
		makeDir(dir);
		return dir;
	}

	virtual QDir getTempDir () const
	{
		static QString tempDir = getCurWorkDir().path() + "/.temp/";
		makeDir(tempDir);
		return tempDir;
	}

	virtual bool makeDir (const QString& name) const
	{
		QDir pwd = getCurWorkDir();
		return (pwd.mkdir(name) != -1);
	}

	virtual bool removeDir (const QString& dirName) const
	{
		bool result = false;
		QDir dir(dirName);
		if (dir.exists(dirName)) {
			Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
				result = info.isDir() ? removeDir(info.absoluteFilePath()) : QFile::remove(info.absoluteFilePath());
				if (!result)
				return result;
			}
			result = dir.rmdir(dirName);
		}
		return result;
	}

	virtual void exit (const QString& reason, int errorCode) const
	{
		if (errorCode != EXIT_SUCCESS) {
			error(reason);
		} else {
			debug(reason + " => error code " + QString::number(errorCode));
		}
		::exit(errorCode);
	}
};
