/*
 * AbstractAssetsParser.h
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

#include "common/MemoryAllocator.h"
#include "constants/FileType.h"
#include "constants/Content.h"

#include <map>
#include <vector>

#include <QDir>
#include <QFile>
#include <QString>

class AbstractAssetsParser {
public:
	AbstractAssetsParser ();
	virtual ~AbstractAssetsParser ();
	virtual void parse () = 0;

	void setTargetDir (const QDir& dir);
	void setTempDir (const QDir& dir);
	void setUseVector (const bool use);
	void init ();

protected:
	struct AssetBit {
		QString name;
		QString path;
		QString x;
		QString y;
		QString alpha;
		QString visible;
		virtual ~AssetBit ()
		{
		}
	};

	struct Asset: public AssetBit {
		Content::Class clazz;
		virtual ~Asset ()
		{
		}
	};

	typedef std::vector<AssetBit> ImageList;
	typedef ImageList::const_iterator ImageListConstIter;

	struct SpriteAsset: public Asset {
		ImageList assets;
	};

	typedef std::map<QString, QFile*> TemplateList;
	typedef TemplateList::iterator TemplateListIter;

	typedef std::vector<QString> CompileList;
	typedef CompileList::const_iterator CompileListConstIter;

	typedef class WriteFile: public IMemoryAllocationObject, public QFile {
		size_t msize () const
		{
			return sizeof(*this);
		}
	} WriteFile;

	QFile* createEmptyFile (const QString& name) const;
	QFile* openTemplateFile (const QString& name);

	void createMainClass ();
	void createExtSpriteClass (Content::Class clazz);
	void createFileCommon (const QString& name, const QString& path);
	void createFileSprite (const SpriteAsset* asset);

	inline Content::Class getClassType (const File::Type type) const;
	inline QString getMimeType (const File::Type type) const;

	QDir _targetDir;
	QDir _tempDir;

private:
	void replaceProperty (const QString& variable, QString& line, int index, int* offset) const;

	QString _fileHeader;
	mutable MemoryAllocator _memAllocator;
	TemplateList _templateList;
	CompileList _compileList;
	bool _withsp;
	bool _withmc;
	bool _useVector;
};
