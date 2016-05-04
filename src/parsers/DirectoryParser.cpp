/*
 * DirectoryParser.cpp
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

#include "DirectoryParser.h"
#include "common/Logger.h"
#include "constants/FileType.h"
#include "constants/Content.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>

#include <QByteArray>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

namespace {
const QString PATTERN_DIGITS = "\\d+";
const QRegExp REGEXP_DIGITS(PATTERN_DIGITS);
}

DirectoryParser::DirectoryParser () :
		AbstractAssetsParser(),
		_regExpMc(),
		_regExpSp(),
		_regExpMc0(),
		_regExpSp0(),
		_suffixIgnorePattern(),
		_currDirList(),
		_ignoreHidden(true)
{
}

DirectoryParser::~DirectoryParser ()
{
}

void DirectoryParser::setMovieclipPattern (const QString& pattern)
{
	const int index = pattern.indexOf(PATTERN_DIGITS);
	_regExpMc.setPattern(pattern);
	_regExpMc0.setPattern(QString(pattern).replace(index, PATTERN_DIGITS.length(), QString::number(0)));
}

void DirectoryParser::setSpritePattern (const QString& pattern)
{
	const int index = pattern.indexOf(PATTERN_DIGITS);
	_regExpSp.setPattern(pattern);
	_regExpSp0.setPattern(QString(pattern).replace(index, PATTERN_DIGITS.length(), QString::number(0)));
}

void DirectoryParser::setSuffixIgnorePattern (const QString& pattern)
{
	_suffixIgnorePattern = pattern;
}

void DirectoryParser::parse ()
{
	init();
	QElapsedTimer etime;
	etime.start();
	parse(_targetDir);
	createMainClass();
	info("parsing took " + QString::number(etime.elapsed() / (float) 1000) + " seconds");
}

void DirectoryParser::parse (QDir& dir)
{
	info("parsing: " + dir.path());
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::Name);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		const QFileInfo& fileInfo = list.at(i);
		if (fileInfo.isDir()) {
			QDir sdir(fileInfo.filePath());
			parse(sdir);
		} else {
			_currDirList = list;
			createAssetFile(fileInfo);
		}
	}

}

void DirectoryParser::createAssetFile (const QFileInfo& fileInfo)
{
	QString name = fileInfo.completeBaseName();
	removeIgnoredSuffix(name);

	if (!_regExpMc.isEmpty() && _regExpMc.indexIn(name) > -1) {
		if (_regExpMc0.indexIn(name) > -1) {
			parseSpriteImageList(QString(name).remove(_regExpMc), Content::MOVIECLIP);
		}
	} else if (!_regExpSp.isEmpty() && _regExpSp.indexIn(name) > -1) {
		if (_regExpSp0.indexIn(name) > -1) {
			parseSpriteImageList(QString(name).remove(_regExpSp), Content::SPRITE);
		}
	} else {
		createFileCommon(name, _tempDir.relativeFilePath(fileInfo.filePath()));
	}
}

void DirectoryParser::parseSpriteImageList (const QString& name, const Content::Class clazz)
{
	QRegExp regExp;
	switch (clazz) {
	case Content::SPRITE:
		regExp = _regExpMc;
		break;
	case Content::MOVIECLIP:
		regExp = _regExpSp;
		break;
	case Content::UNDEFINED:
	default:
		warning("undefined class type for " + name);
		return;
	}

	SpriteAsset sprite;
	const QRegExp rx(QString(name).replace(regExp, regExp.pattern()));

	for (int i = 0; i < _currDirList.size(); ++i) {
		const QFileInfo fileInfo = _currDirList.at(i);
		const QString fname = fileInfo.fileName();

		int index = 0;
		if (fileInfo.isFile() && (index = rx.indexIn(fname)) > -1) {
			if (fname.at(index + rx.matchedLength()).digitValue() > 0) {
				continue;
			}
			const int ri = regExp.indexIn(fname);
			const int rm = regExp.matchedLength();
			const QString matched = fname.mid(ri, rm);
			const int ni = REGEXP_DIGITS.indexIn(matched);
			const int nm = REGEXP_DIGITS.matchedLength();
			const unsigned int index = matched.mid(ni, nm).toInt();
			AssetBit asset;
			asset.path = _tempDir.relativeFilePath(fileInfo.filePath());
			if (sprite.assets.size() < index + 1) {
				sprite.assets.resize(index + 1);
			}
			sprite.assets[index] = asset;
		}
	}
	sprite.name = QString(name).remove(regExp);
	sprite.clazz = clazz;
	AbstractAssetsParser::createFileSprite(&sprite);
}

void DirectoryParser::removeIgnoredSuffix (QString& name) const
{
	if (_suffixIgnorePattern.isEmpty())
		return;
	int index = name.lastIndexOf(_suffixIgnorePattern);
	if (index > -1) {
		name.replace(index, name.length() - index, "");
	}
}
