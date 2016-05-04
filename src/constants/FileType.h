/*
 * FileType.h
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

#include <QString>

namespace File {
typedef enum Type {
	UNSUPPORTED = 0, PNG, JPG, GIF, BMP, WAV, MP3, OGG, SWF, XML, TXT, JSON, BIN
} Type;

inline Type getType (const QString& type)
{
	int index = type.lastIndexOf(".");
	if (index < 0) return BIN;

	const char* ext = type.toLower().mid(index + 1).toStdString().c_str();

	if (strcmp(ext, "png") == 0) return PNG;
	else if (strcmp(ext, "jpg") == 0) return JPG;
	else if (strcmp(ext, "gif") == 0) return GIF;
	else if (strcmp(ext, "bmp") == 0) return BMP;
	else if (strcmp(ext, "wav") == 0) return WAV;
	else if (strcmp(ext, "mp3") == 0) return MP3;
	else if (strcmp(ext, "ogg") == 0) return OGG;
	else if (strcmp(ext, "swf") == 0) return SWF;
	else if (strcmp(ext, "xml") == 0) return XML;
	else if (strcmp(ext, "txt") == 0) return TXT;
	else if (strcmp(ext, "json") == 0) return JSON;
	else if (strcmp(ext, "exe") == 0) return BIN;

	return UNSUPPORTED;
}
}
