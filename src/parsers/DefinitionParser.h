/*
 * DefinitionParser.h
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

#include "AbstractAssetsParser.h"
#include "constants/CompileMode.h"
#include "constants/Content.h"

#include <map>
#include <vector>

#include <QDomDocument>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QFileInfo>
#include <QString>

#define DEFINITION_NAME "definition.xml"

class DefinitionParser: public AbstractAssetsParser {
public:
	DefinitionParser ();
	~DefinitionParser ();

	void parse ();

	struct CompileArguments {
		QString name;
		float player;
		CompileMode::Mode mode;
		int quality;
		bool swc;
	};

	bool getCompileArguments (CompileArguments& definition);

private:
	void parseAssetNodes (QDomNode& node, const Content::Class clazz);
	bool createSingleFrameAsset (QDomNode& node, const Content::Class clazz, const QString& tag);
	bool createMultiFrameSprite (QDomNode& frame, const Content::Class clazz);
	void copyAttributes (AssetBit* asset, const QDomNamedNodeMap& attributes) const;

	inline void checkAttributes (QDomNode& node) const;
	inline bool checkPathExists (const QString& path) const;
	inline void warnInvalidTag (const QString& tag, const QString& parent) const;
	inline void warnMissingAttr (const QString& attr, const QString& tag) const;

	QDomDocument _definition;
	mutable std::map<QString, int> _attributesMap;
	std::map<QString, Asset*> _assets;
};
