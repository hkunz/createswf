/*
 * Content.h
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

namespace Content {

const QString STR_MAIN = "Main";
const QString STR_EXTMOVIECLIP = "ExtendedMovieClip";
const QString STR_EXTSPRITE = "ExtendedSprite";
const QString STR_MOVIECLIP = "MovieClip";
const QString STR_SPRITE = "Sprite";
const QString STR_BITMAPDATA = "BitmapData";
const QString STR_SOUND = "Sound";
const QString STR_BYTEARRAY = "ByteArray";
const QString STR_DOT_AS = ".as";
const QString STR_DOT_SWF = ".swf";
const QString STR_DOT_SWC = ".swc";

typedef enum Class {
	UNDEFINED = 0, SPRITE, MOVIECLIP, EXTSPRITE, EXTMOVIECLIP, BITMAPDATA, SOUND, BYTEARRAY
} Class;

inline const QString getContentName (Class clazz)
{
	switch (clazz) {
	case EXTSPRITE: return STR_EXTSPRITE;
	case EXTMOVIECLIP: return STR_EXTMOVIECLIP;
	case SPRITE: return STR_SPRITE;
	case MOVIECLIP:	return STR_MOVIECLIP;
	case BITMAPDATA: return STR_BITMAPDATA;
	case SOUND:	return STR_SOUND;
	case BYTEARRAY:	return STR_BYTEARRAY;
	default: return "[invalid]";
	}
}
}
