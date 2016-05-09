CreateSWF - http://www.createswf.org/
======================================================

About CreateSWF
======================================================

CreateSWF is a tool to compile assets contained in a target folder directly
into an SWF file. It is meant to be used for creating runtime SWF libraries
without the hassle of using a Flash IDE to manually import assets. CreateSWF
was originally written in Perl and currently ported to C++. To use it, make
sure that the environment variable FLEX_HOME is defined and pointing to the
Flex SDK.

Compiling
======================================================

Make sure "cmake" and the "Qt5" development libraries are installed then run:

    $ mkdir -p build
    $ cd build
    $ cmake ../
    $ make

Tool Usage
======================================================

Before using this tool, you must make sure that the environment variable
FLEX_HOME is defined and pointing to the location of the Flex SDK

	==> FOR MAC OS X USER:

	Mac OS X is somewhat crappy with consistency of environment variables, when you
	run the application in GUI mode, you need to have a "environment.plist" file in
	directory ~/.MacOSX/environment.plist which must contain the path to FLEX_HOME.
	Put the contents with the path of Flex SDK in environment.plist file like this
	with the curly braces:

	{
		"FLEX_HOME" = "/path/to/your/flexsdk/";
	}
	
	Save that, then you must log out or restart for the changes to take effect.

	==> FOR WINDOWS USER: Just google how to add environment variables.
	==> FOR LINUX FOLKS: I don't need to explain :)

You can run the tool through command line or run it via UI.
If you run it via the command line use this syntax:

./createswf [options] directory

This tool has 3 modes:

(1)	Embed all assets in a target directory (option: --mode=1)

	This mode is the default mode, and takes all assets in the target directory,
	including sub-directories and embeds all supported files in a SWF (or SWC
	using option --swc=1) A file named "mysound.mp3" will have the class name "mysound"
	and will use "Sound" as the base class. A file named "mypic.png" will have the
	class "mypic" and will extend the base class "BitmapData". All binaries/text files
	will extend "ByteArray". You can also compile MovieClips and Sprites by prepending
	the image (representing one frame) with "mcX__classname" (or "spX__classname" for
	sprites) where X starts at 0 representing frame zero. and "classname" is the class
	name for the movieclip or sprite.

(2)	Parse a definition.xml file in the target directory

	This mode (using option --mode=2) will take a file named "definition.xml" in
	the target directory and parse the document to see which assets have to be
	embedded. A sample xml can be found under code scripts/definition.xml and it's DTD.
	If this mode is specified but no definition.xml is found, the default --mode=1
	will be used.

(3)	This mode is not yet available and will come with release 0.2. This is a combination
	of modes 1 and 2 in that it compiles all assets in the target directory and searches
	if for the corresponding asset file any properties (x,y,alpha,visible,etc) are
	available to use.

More information as of the usage can also be found in the original perl script
under code scripts/createswf.pl. You can see the man-page at the bottom of the file
or run "perldoc createswf.pl" in the command line.
