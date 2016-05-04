/*
 * DefinitionParser.cpp
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

#include "DefinitionParser.h"
#include "common/Logger.h"
#include "constants/DefinitionNode.h"
#include "constants/CompileMode.h"
#include "ports/System.h"

#include <stdio.h>
#include <stdlib.h>

#include <QDomDocument>
#include <QFile>

namespace {
const QString NODE_SPRITES = "sprites";
const QString NODE_MOVIECLIPS = "movieclips";
const QString NODE_BITMAPS = "bitmaps";
const QString NODE_SOUNDS = "sounds";
const QString NODE_BINARIES = "binaries";
const QString NODE_SPRITE = "spr";

const QString NODE_MOVIECLIP = "mc";
const QString NODE_BITMAP = "bmp";
const QString NODE_SOUND = "snd";
const QString NODE_BINARY = "bin";
const QString NODE_OBJECT = "object";
const QString NODE_FRAME = "frame";

const QString ATTR_CLASS = "class";
const QString ATTR_PATH = "path";
const QString ATTR_NAME = "name";
const QString ATTR_X = "x";
const QString ATTR_Y = "y";
const QString ATTR_ALPHA = "alpha";
const QString ATTR_VISIBLE = "visible";
}

DefinitionParser::DefinitionParser () :
		_definition(DEFINITION_NAME), _attributesMap(), _assets()
{

}

DefinitionParser::~DefinitionParser ()
{
}

bool DefinitionParser::getCompileArguments (CompileArguments& definition)
{
	QFile file(_targetDir.filePath(DEFINITION_NAME));
	if (!file.open(QIODevice::ReadOnly)) {
		return false;
	}
	QString error;
	if (!_definition.setContent(&file, true, &error)) {
		file.close();
		System.exit(QString(DEFINITION_NAME) + " => " + error, EXIT_FAILURE);
	}
	file.close();

	QDomElement doc = _definition.documentElement();
	QDomNode node = doc.firstChild();
	const QString root = doc.toElement().tagName();
	float player = 11.1;
	int quality = 100;
	bool hasLib = false;
	CompileMode::Mode mode = CompileMode::COMPILE_DEFINITION;

	while (!node.isNull()) {
		QDomElement e = node.toElement();
		node = node.nextSibling();

		if (e.isNull()) {
			continue;
		}

		const QString text = e.text();
		const QString tag = e.tagName();
		if (tag == DefinitionNode::LIBRARY) {
			hasLib = true;
		} else if (tag == DefinitionNode::MODE) {
			mode = CompileMode::Mode(text.toInt());
		} else if (tag == DefinitionNode::PLAYER) {
			player = text.toFloat();
		} else if (tag == DefinitionNode::QUALITY) {
			quality = text.toInt();
			if (quality < 0 || quality > 100)
				quality = 100;
		} else if (tag == DefinitionNode::NAME) {
			if (definition.name.isEmpty()) {
				definition.name = text;
			}
		} else if (tag == DefinitionNode::SWC) {
			if(text.toInt() == 1) {
				definition.swc = true;
			}
		} else {
			warnInvalidTag(tag, root);
		}
	}

	if (definition.mode == CompileMode::UNDEFINED) {
		definition.mode = hasLib ? mode : CompileMode::COMPILE_ALL;
	}
	if (definition.player < 0) {
		definition.player = player;
	}
	if (definition.quality < 0) {
		definition.quality = quality;
	}

	return true;
}

void DefinitionParser::parse ()
{
	info("parse definition.xml");
	init();

	_attributesMap[ATTR_CLASS] = 1;
	_attributesMap[ATTR_PATH] = 1;
	_attributesMap[ATTR_NAME] = 1;
	_attributesMap[ATTR_X] = 1;
	_attributesMap[ATTR_Y] = 1;
	_attributesMap[ATTR_ALPHA] = 1;
	_attributesMap[ATTR_VISIBLE] = 1;

	QDomElement doc = _definition.documentElement();
	QDomNode node = doc.firstChildElement(DefinitionNode::LIBRARY).firstChild();

	while (!node.isNull()) {
		QDomElement elem = node.toElement();
		node = node.nextSibling();

		if (elem.isNull()) {
			continue;
		}

		const QString text = elem.text();
		const QString tag = elem.tagName();

		QDomNode n(elem.firstChild());
		if (tag == ::NODE_SPRITES) {
			parseAssetNodes(n, Content::SPRITE);
		} else if (tag == ::NODE_MOVIECLIPS) {
			parseAssetNodes(n, Content::MOVIECLIP);
		} else if (tag == ::NODE_BITMAPS) {
			parseAssetNodes(n, Content::BITMAPDATA);
		} else if (tag == ::NODE_SOUNDS) {
			parseAssetNodes(n, Content::SOUND);
		} else if (tag == ::NODE_BINARIES) {
			parseAssetNodes(n, Content::BYTEARRAY);
		} else {
			warnInvalidTag(tag, node.parentNode().nodeName());
		}
	}

	for (std::map<QString, Asset*>::const_iterator i = _assets.begin(); i != _assets.end(); ++i) {
		const Asset* asset = i->second;
		if (asset->clazz == Content::MOVIECLIP || asset->clazz == Content::SPRITE) {
			AbstractAssetsParser::createFileSprite(static_cast<const SpriteAsset*>(asset));
		} else {
			AbstractAssetsParser::createFileCommon(asset->name, asset->path);
		}
		delete asset;
	}
	createMainClass();
}

void DefinitionParser::parseAssetNodes (QDomNode& node, const Content::Class clazz)
{
	bool sprite = false;
	QString nodeName;
	switch (clazz) {
	case Content::MOVIECLIP:
		nodeName = NODE_MOVIECLIP;
		sprite = true;
		break;
	case Content::SPRITE:
		sprite = true;
		nodeName = NODE_SPRITE;
		break;
	case Content::BITMAPDATA:
		nodeName = NODE_BITMAP;
		break;
	case Content::SOUND:
		nodeName = NODE_SOUND;
		break;
	case Content::BYTEARRAY:
		nodeName = NODE_BINARY;
		break;
	default:
		error("invalid class type " + QString::number(clazz));
		return;
	}
	while (!node.isNull()) {
		QDomElement elem = node.toElement();
		QDomNode parseNode = node;
		QDomNamedNodeMap attributes = node.attributes();
		QDomNode frame = node.firstChild();
		checkAttributes(node);
		node = node.nextSibling();

		if (elem.isNull()) {
			continue;
		}

		const QString tag = elem.tagName();
		if (nodeName != tag) {
			warnInvalidTag(tag, parseNode.parentNode().nodeName());
			continue;
		}

		if (attributes.isEmpty() || attributes.namedItem(ATTR_CLASS).nodeValue().isEmpty()) {
			warnMissingAttr(ATTR_CLASS, tag);
			continue;
		}

		if (frame.isNull() || !sprite) {
			createSingleFrameAsset(parseNode, clazz, nodeName);
		} else {
			createMultiFrameSprite(frame, clazz);
		}
	}
}

bool DefinitionParser::createSingleFrameAsset (QDomNode& node, const Content::Class clazz, const QString& tag)
{
	QDomNamedNodeMap attr = node.attributes();
	const QString name = attr.namedItem(ATTR_CLASS).nodeValue();
	const QString pathattr = attr.namedItem(ATTR_PATH).nodeValue();
	if (pathattr.isEmpty()) {
		warnMissingAttr(ATTR_PATH, tag);
		return false;
	}
	const QString path = _targetDir.absoluteFilePath(pathattr);
	if (!checkPathExists(path)) {
		return false;
	}
	if (name.isEmpty()) {
		warnMissingAttr(ATTR_CLASS, tag);
		return false;
	}
	if (clazz == Content::SPRITE || clazz == Content::MOVIECLIP) {
		struct AssetBit asset;
		asset.path = _tempDir.relativeFilePath(path);
		copyAttributes(&asset, attr);
		struct SpriteAsset* sprite = new SpriteAsset();
		sprite->assets.push_back(asset);
		sprite->clazz = clazz;
		sprite->name = name;
		_assets[name] = sprite;
	} else {
		struct Asset* common = new Asset();
		common->path = path;
		common->name = name;
		_assets[name] = common;
	}
	return true;
}

bool DefinitionParser::createMultiFrameSprite (QDomNode& frame, const Content::Class clazz)
{
	QDomNode parent = frame.parentNode();
	QDomNamedNodeMap attributes = parent.attributes();
	const QString childName = clazz == Content::MOVIECLIP ? ::NODE_FRAME : ::NODE_OBJECT;
	const QString className = attributes.namedItem(ATTR_CLASS).nodeValue();
	const QString basePath = attributes.namedItem(ATTR_PATH).nodeValue();
	const QString absBasePath = _targetDir.absoluteFilePath(basePath);
	struct SpriteAsset* sprite = NULL;

	if (!QFile::exists(absBasePath)) {
		info("base path \'" + absBasePath + "\' does not exist");
		return false;
	}

	while (!frame.isNull()) {
		QDomElement e = frame.toElement();
		QDomNamedNodeMap attr = frame.attributes();
		checkAttributes(frame);
		frame = frame.nextSibling();

		if (e.isNull()) {
			continue;
		}

		const QString tn = e.tagName();
		if (tn != childName) {
			warnInvalidTag(tn, frame.parentNode().nodeName());
			continue;
		}

		const QString relpath = attr.namedItem(ATTR_PATH).nodeValue();
		if (attr.isEmpty() || relpath.isEmpty()) {
			warnMissingAttr(ATTR_PATH, tn);
			continue;
		}

		const QString path = _targetDir.absoluteFilePath(basePath + relpath);
		if (!checkPathExists(path)) {
			continue;
		}
		struct AssetBit asset;
		asset.name = attr.namedItem(ATTR_NAME).nodeValue();
		asset.path = _tempDir.relativeFilePath(path);
		copyAttributes(&asset, attr);
		if (!sprite) {
			sprite = new SpriteAsset();
		}
		sprite->assets.push_back(asset);
	}
	if (sprite) {
		sprite->clazz = clazz;
		sprite->name = className;
		copyAttributes(sprite, attributes);
		_assets[className] = sprite;
	}
	return true;
}

void DefinitionParser::copyAttributes (AssetBit* asset, const QDomNamedNodeMap& attributes) const
{
	asset->x = attributes.namedItem(ATTR_X).nodeValue();
	asset->y = attributes.namedItem(ATTR_Y).nodeValue();
	asset->alpha = attributes.namedItem(ATTR_ALPHA).nodeValue();
	asset->visible = attributes.namedItem(ATTR_VISIBLE).nodeValue();
}

inline void DefinitionParser::checkAttributes (QDomNode& node) const
{
	const QString tag = node.toElement().tagName();
	QDomNamedNodeMap attr = node.attributes();
	for (int i = 0; i < attr.size(); ++i) {
		QDomNode a = attr.item(i);
		if (!_attributesMap[a.nodeName()]) {
			warning("invalid attribute \'" + a.nodeName() + "\' in \'" + tag + "\'");
		}
	}
}

bool DefinitionParser::checkPathExists (const QString& path) const
{
	const bool exists = QFile::exists(path);
	if (!exists) {
		warning("path \'" + path + "\' does not exist");
	}
	return exists;
}

void DefinitionParser::warnInvalidTag (const QString& tag, const QString& parent) const
{
	warning("invalid node \'" + tag + "\' in parent \'" + parent + "\' within " + DEFINITION_NAME);
}

void DefinitionParser::warnMissingAttr (const QString& attr, const QString& tag) const
{
	warning("missing \'" + attr + "\' attribute in \'" + tag + "\' node");
}
