/*
 * CommandLineParser.cpp
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

#include "CommandLineParser.h"

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

CommandLineParser::CommandLineParser () :
	_target(NULL),
	_output(NULL),
	_player(-1),
	_verbosity(1),
	_quality(-1),
	_mode(CompileMode::UNDEFINED),
	_swc(false),
	_debug(false)
{
}

CommandLineParser::~CommandLineParser ()
{

}

bool CommandLineParser::parse (int argc, char* argv[])
{
	extern int opterr;
	extern char* optarg;

	opterr = 0;

	static struct option options[] = {
			{ "mode", 1, 0, 'm' },
			{ "quality", 1, 0, 'q' },
			{ "output", 1, 0, 'o' },
			{ "verbosity", 0, 0, 'v' },
			{ "swc", 0, 0, 's' },
			{ "debug", 0, 0, 'd' },
			{ "player", 1, 0, 'p' }
	};

	while (1) {
		int index = 0;
		int c = getopt_long(argc, argv, "m:vdsq:o:", options, &index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 'd':
			_debug = true;
			break;

		case 'v':
			++_verbosity;
			break;

		case 'm': {
			int mode = atoi(optarg);
			if (!CompileMode::checkMode(mode)) {
				printf("invalid mode\n");
				return EXIT_FAILURE;
			}
			_mode = CompileMode::Mode(mode);
			printf("using mode %d\n", _mode);
			break;
		}

		case 'p': {
			int player = atof(optarg);
			if (player < 9) {
				printf("invalid quality %d\n", player);
				return EXIT_FAILURE;
			}
			_player = player;
			printf("option p with value `%.1f'\n", _player);
			break;
		}

		case 'q': {
			int quality = atoi(optarg);
			if (quality < 1 || quality > 100) {
				printf("invalid quality %d\n", quality);
				return EXIT_FAILURE;
			}
			_quality = quality;
			printf("option q with value `%i'\n", _quality);
			break;
		}

		case 'o':
			_output = optarg;
			printf("option o with value `%s'\n", _output);
			break;

		case 's':
			_swc = true;
			printf("option s with value `%d'\n", _swc);
			break;

		case '?':
			switch (optopt) {
			case 'm':
			case 'q':
			case 'o':
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				break;
			default:
				if (isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				}
				break;
			}
			return EXIT_FAILURE;
			break;

		default:
			printf("?? getopt returned character code 0%o ??\n", c);
			break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc) {
			_target = argv[optind++];
			printf("%s ", _target);
			break;
		}
		printf("\n");
	}

	return EXIT_SUCCESS;
}

char* CommandLineParser::getTargetDir () const
{
	return _target;
}

char* CommandLineParser::getOutput () const
{
	return _output;
}

float CommandLineParser::getPlayer () const
{
	return _player;
}

int CommandLineParser::getVerbosityLevel () const
{
	return _verbosity;
}

int CommandLineParser::getQuality () const
{
	return _quality;
}

CompileMode::Mode CommandLineParser::getCompileMode () const
{
	return _mode;
}

bool CommandLineParser::isSWC () const
{
	return _swc;
}

bool CommandLineParser::isDebug () const
{
	return _debug;
}
