/*
 * AboutDialog.cpp
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

#include "AboutDialog.h"

#include <QCoreApplication>

using namespace CreateSWF::Internal;

AboutDialog::AboutDialog (QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	const QString html = QCoreApplication::translate("AboutDialog",
		"<p align=\"center\"><font size=\"+2\"><b>CreateSWF</b></font><br><i>Version %1</i></p>\n"
		"<p align=\"center\">Copyright 2012 Harry Kunz</p>\n"
		"<p align=\"center\">You may modify and redistribute this program under the terms of the GPL (version 2 or later). "
		"A copy of the GPL is contained in the 'COPYING' file distributed with CreateSWF.</p>\n"
		"<p align=\"center\"><a href=\"http://www.createswf.org/\">http://www.createswf.org/</a></p>\n")
	.arg(QApplication::applicationVersion());
	textBrowser->setHtml(html);
}
