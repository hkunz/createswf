/*
 * AbstractAssetsParser.cpp
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

#include "AbstractAssetsParser.h"
#include "common/Logger.h"
#include "common/Version.h"
#include "constants/Content.h"

#include <string.h>

#include <QDateTime>
#include <QFile>
#include <QRegExp>

namespace {
const QString VAR_WIDTH = "${width}";
const QString VAR_HEIGHT = "${height}";
const QString VAR_BGCOLOR = "${bgcolor}";
const QString VAR_FPS = "${fps}";
const QString VAR_NAME = "${name}";
const QString VAR_PATH = "${path}";
const QString VAR_MIME = "${mime}";
const QString VAR_LOOP1 = "${loop1}";
const QString VAR_LOOP2 = "${loop2}";
const QString VAR_COUNT = "${count}";
const QString VAR_X = "${x}";
const QString VAR_Y = "${y}";
const QString VAR_ALPHA = "${alpha}";
const QString VAR_VISIBLE = "${visible}";
const QString VAR_ARRAYTYPE = "${arraytype}";

const QString STR_TRUE = "true";

QRegExp REGEXP_VARIABLE("\\$\\{.+\\}");

const size_t MEMORY = 24;
}

AbstractAssetsParser::AbstractAssetsParser () :
	_targetDir(),
	_tempDir(),
	_fileHeader("//\n// "),
	_memAllocator(MEMORY),
	_templateList(),
	_compileList()
{
	REGEXP_VARIABLE.setMinimal(true);

	_withsp = false;
	_withmc = false;
	_useVector = true;
}

AbstractAssetsParser::~AbstractAssetsParser ()
{
	for (TemplateListIter i = _templateList.begin(); i != _templateList.end(); ++i) {
		debug("closing read stream " + i->first);
		QFile* readable = i->second;
		readable->close();
		delete readable;
	}
}

void AbstractAssetsParser::setTargetDir (const QDir& dir)
{
	_targetDir = dir;
}

void AbstractAssetsParser::setTempDir (const QDir& dir)
{
	_tempDir = dir;
}

void AbstractAssetsParser::setUseVector (const bool use)
{
	_useVector = use;
}

void AbstractAssetsParser::init ()
{
	_fileHeader.append(APPFULLNAME);
	_fileHeader.append("\n// ");
	_fileHeader.append(COPYRIGHT);
	_fileHeader.append("\n//\n// Automatically generated on ");
	_fileHeader.append(QDateTime::currentDateTime().toString());
	_fileHeader.append("\n//\n\n");
}

QFile* AbstractAssetsParser::createEmptyFile (const QString& name) const
{
	WriteFile* handle = new (_memAllocator) WriteFile();
	handle->setFileName(_tempDir.absoluteFilePath(name));

	if (!handle->open(QIODevice::WriteOnly)) {
		error("failed to open file " + handle->fileName());
		operator delete(handle, _memAllocator);
		return NULL;
	}

	handle->write(_fileHeader.toStdString().c_str());
	return handle;
}

QFile* AbstractAssetsParser::openTemplateFile (const QString& name)
{
	QFile* readable = NULL;
	if ((readable = _templateList[name])) {
		readable->seek(0);
		return readable;
	}
	readable = new QFile(name);
	if (!readable->open(QIODevice::ReadOnly | QIODevice::Text)) {
		error("could not open template file " + readable->fileName());
		delete readable;
		return NULL;
	}
	debug("open read stream " + name);
	return _templateList[name] = readable;
}

void AbstractAssetsParser::createMainClass ()
{
	const QString name = Content::STR_MAIN + Content::STR_DOT_AS;
	QFile* readable = NULL;
	QFile* writable = NULL;

	if (!(readable = openTemplateFile(":" + name)) || !(writable = createEmptyFile(name))) {
		return;
	}

	info("created " + QString::number(_compileList.size()) + " asset files");
	info("creating main class: " + name);

	std::map<QString, const QString*> varmap;
	const QString size = "1";
	const QString bgcolor = "#ffffff";
	const QString fps = "24";
	const QString loop = "";

	varmap[::VAR_WIDTH] = &size;
	varmap[::VAR_HEIGHT] = &size;
	varmap[::VAR_BGCOLOR] = &bgcolor;
	varmap[::VAR_FPS] = &fps;
	varmap[::VAR_LOOP1] = &loop;

	while (!readable->atEnd()) {
		QString line = readable->readLine();
		int offset = 0;
		int idx;

		while ((idx = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
			const int len = REGEXP_VARIABLE.matchedLength();
			const QString var = line.mid(idx, len);
			const QString* value = varmap[var];
			if (value) {
				line.remove(idx, len);
				line.insert(idx, *value);
				offset = idx + value->length();
			} else {
				warning("undefined variable " + var);
				offset = idx + len;
				continue;
			}

			if (var == ::VAR_LOOP1) {
				int ix;
				int offsetCount = 0;
				int offsetName = 0;
				offset = 0;

				while ((ix = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
					const int len = REGEXP_VARIABLE.matchedLength();
					const QString subvar = line.mid(ix, len);
					line.remove(ix, len);
					if (subvar == ::VAR_COUNT) {
						offsetCount = ix;
					} else {
						offsetName = ix;
					}
				}

				int count = 0;
				for (CompileListConstIter i = _compileList.begin(); i != _compileList.end(); ++i) {
					QString loopLine(line);
					QString strcount = QString::number(count++);
					loopLine.insert(offsetCount, strcount);
					loopLine.insert(offsetName + strcount.length(), *i);
					writable->write(loopLine.toStdString().c_str(), loopLine.length());
				}
				line.truncate(0);
			}
		}
		writable->write(line.toStdString().c_str(), line.length());
	}
	writable->close();
	operator delete(static_cast<WriteFile*> (writable), _memAllocator);

	if (_withsp)
		createExtSpriteClass(Content::EXTSPRITE);

	if (_withmc)
		createExtSpriteClass(Content::EXTMOVIECLIP);
}

void AbstractAssetsParser::createExtSpriteClass (Content::Class clazz)
{
	const QString name = Content::getContentName(clazz) + Content::STR_DOT_AS;
	QFile* readable = NULL;
	QFile* writable = NULL;

	if (!(readable = openTemplateFile(":" + name)) || !(writable = createEmptyFile(name))) {
		return;
	}

	info("creating base class: " + name);

	int found = 0;
	int replace = 2;
	const QString arrayType = _useVector ? "Vector.<DisplayObject>" : "Array";
	static std::map<QString, const QString*> varmap;

	varmap[::VAR_ARRAYTYPE] = &arrayType;

	while (!readable->atEnd()) {
		QString line = readable->readLine();
		int offset = 0;
		int idx;

		const bool parse = found < replace && clazz != Content::EXTSPRITE;
		while (parse && (idx = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
			const int len = REGEXP_VARIABLE.matchedLength();
			const QString var = line.mid(idx, len);
			const QString* value = varmap[var];
			if (value) {
				line.remove(idx, len);
				line.insert(idx, *value);
				offset = idx + value->length();
				++found;
			} else {
				warning("undefined variable " + var);
				offset = idx + len;
			}
		}
		writable->write(line.toStdString().c_str(), line.length());
	}
	writable->close();
	operator delete(static_cast<WriteFile*> (writable), _memAllocator);
}

void AbstractAssetsParser::createFileCommon (const QString& name, const QString& path)
{
	File::Type type = File::getType(path);
	if (type == File::UNSUPPORTED) {
		warning("unsupported file type: " + path);
		return;
	}

	const QString baseName = Content::getContentName(getClassType(type));
	QFile* readable = openTemplateFile(":" + baseName + Content::STR_DOT_AS);
	if (!readable) {
		return;
	}

	QFile* writable = createEmptyFile(name + Content::STR_DOT_AS);
	if (writable == NULL) {
		return;
	}

	info("creating: " + writable->fileName() + " of type \'" + baseName + "\'");
	_compileList.push_back(name);

	static std::map<QString, const QString*> varmap;
	const QString mime = getMimeType(type);

#ifdef __WIN32__
	QString npath = QString(path).replace("\\", "/");
	varmap[::VAR_PATH] = &npath;
#else
	varmap[::VAR_PATH] = &path;
#endif
	varmap[::VAR_MIME] = &mime;
	varmap[::VAR_NAME] = &name;

	while (!readable->atEnd()) {
		QString line = readable->readLine();
		int offset = 0;
		int idx;

		while ((idx = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
			const int len = REGEXP_VARIABLE.matchedLength();
			const QString var = line.mid(idx, len);
			const QString* value = varmap[var];
			if (value) {
				line.remove(idx, len);
				line.insert(idx, *value);
				offset = idx + value->length();
			} else {
				warning("undefined variable " + var);
				offset = idx + len;
			}
		}
		writable->write(line.toStdString().c_str(), line.length());
	}
	writable->close();
	operator delete(static_cast<WriteFile*> (writable), _memAllocator);
}

void AbstractAssetsParser::createFileSprite (const SpriteAsset* asset)
{
	ImageList imageList = asset->assets;
	const QString name = asset->name;
	const QString baseName = Content::getContentName(asset->clazz);

	QFile* readable = openTemplateFile(":" + baseName + Content::STR_DOT_AS);
	if (!readable) {
		return;
	}

	QFile* writable = createEmptyFile(name + Content::STR_DOT_AS);
	if (writable == NULL) {
		return;
	}

	info("creating: " + writable->fileName() + " of type \'" + baseName + "\'");

	const bool ismc = asset->clazz == Content::MOVIECLIP;

	_compileList.push_back(name);
	_withmc |= ismc;
	_withsp |= !ismc;

	while (!readable->atEnd()) {
		QString line = readable->readLine();
		int offset = 0;
		int mi;

		while ((mi = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
			const int len = REGEXP_VARIABLE.matchedLength();
			const QString var = line.mid(mi, len);
			line.remove(mi, len);

			if (var == ::VAR_NAME) {
				line.insert(mi, name);
				offset = mi + name.length();
			} else if (var == ::VAR_LOOP1) {
				int ix;
				int offsetPath = 0;
				int offsetMime = 0;
				int offsetCount = 0;
				offset = 0;

				while ((ix = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
					const int len = REGEXP_VARIABLE.matchedLength();
					const QString subvar = line.mid(ix, len);
					line.remove(ix, len);
					if (subvar == ::VAR_PATH) {
						offsetPath = ix;
					} else if (subvar == ::VAR_MIME) {
						offsetMime = ix;
					} else {
						offsetCount = ix;
					}
				}

				int count = 0;
				for (ImageListConstIter iter = imageList.begin(); iter != imageList.end(); ++iter) {
					AssetBit asset = *iter;
#ifdef __WIN32__
					QString path = asset.path.replace("\\", "/");
#else
					QString path(asset.path);
#endif
					QString loopLine = QString(line);
					const QString mime = getMimeType(File::getType(path));
					loopLine.insert(offsetPath, path);
					loopLine.insert(offsetMime + path.length(), mime);
					loopLine.insert(offsetCount + path.length() + mime.length(), QString::number(count++));
					writable->write(loopLine.toStdString().c_str(), loopLine.length());
				}
				line.truncate(0);
			} else if (var == ::VAR_LOOP2) {
				int ix;
				int offsetCount = 0;
				offset = 0;

				while ((ix = REGEXP_VARIABLE.indexIn(line, offset)) > -1) {
					const int len = REGEXP_VARIABLE.matchedLength();
					const QString subvar = line.mid(ix, len);
					line.remove(ix, len);
					if (subvar == ::VAR_COUNT) {
						offsetCount = ix;
						offset = name.length();
						break;
					}
				}

				int count = 0;
				for (ImageListConstIter iter = imageList.begin(); iter != imageList.end(); ++iter) {
					AssetBit asset = *iter;
					QString loopLine = QString(line);
					loopLine.insert(offsetCount, QString::number(count++));
					loopLine.replace(::VAR_VISIBLE, asset.visible.isEmpty() ? ::STR_TRUE : asset.visible);
					loopLine.replace(::VAR_ALPHA, asset.alpha.isEmpty() ? QString::number(1.0) : asset.alpha);
					loopLine.replace(::VAR_Y, asset.y.isEmpty() ? QString::number(0) : asset.y);
					loopLine.replace(::VAR_X, asset.x.isEmpty() ? QString::number(0) : asset.x);
					writable->write(loopLine.toStdString().c_str(), loopLine.length());
				}
				line.truncate(0);
			} else if (var == ::VAR_X) {
				replaceProperty(asset->x, line, mi, &offset);
			} else if (var == ::VAR_Y) {
				replaceProperty(asset->y, line, mi, &offset);
			} else if (var == ::VAR_ALPHA) {
				replaceProperty(asset->alpha, line, mi, &offset);
			} else if (var == ::VAR_VISIBLE) {
				replaceProperty(asset->visible, line, mi, &offset);
			}
		}
		writable->write(line.toStdString().c_str(), line.length());
	}

	writable->close();
	operator delete(static_cast<WriteFile*> (writable), _memAllocator);
}

void AbstractAssetsParser::replaceProperty (const QString& variable, QString& line, int index, int* offset) const
{
	if (variable.isEmpty()) {
		line.truncate(0);
	} else {
		line.insert(index, variable);
		*offset = index + variable.length();
	}
}

Content::Class AbstractAssetsParser::getClassType (const File::Type type) const
{
	switch (type) {
	case File::PNG:
	case File::JPG:
	case File::GIF:
		return Content::BITMAPDATA;
	case File::BMP:
	case File::WAV:
	case File::OGG:
	case File::SWF:
	case File::XML:
	case File::TXT:
	case File::JSON:
	case File::BIN:
		return Content::BYTEARRAY;
	case File::MP3:
		return Content::SOUND;
	case File::UNSUPPORTED:
	default:
		return Content::UNDEFINED;
	}
}

QString AbstractAssetsParser::getMimeType (const File::Type type) const
{
	switch (type) {
	case File::PNG:
		return "image/png";
	case File::JPG:
		return "image/jpg";
	case File::GIF:
		return "image/gif";
	case File::MP3:
		return "audio/mpeg";
	case File::UNSUPPORTED:
		warning("unsupported file type");
	case File::BMP:
	case File::WAV:
	case File::OGG:
	case File::SWF:
	case File::XML:
	case File::TXT:
	case File::JSON:
	case File::BIN:
	default:
		return "application/octet-stream";
	}
}
