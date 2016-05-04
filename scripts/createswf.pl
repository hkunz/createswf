#!/usr/bin/perl
#
# createswf.pl
# Copyright (c) 2012, Harry Kunz <harry.kunz@ymail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#    3. You must have great looks
#
# THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# hkunz = harry.kunz@ymail.com 06/28/2012
# Script compiles all assets in folders & subfolders into .swf file
#
# usage: ./createswf.pl param
# param = directory path containing assets to compile
# param = or shortcut folder name containing assets to compile in a default directory

use strict;
use warnings;

our(@INC, @ISA, @EXPORT, $VERSION);

$VERSION = "0.2";

die "Environment variable FLEX_HOME is not set\n" unless $ENV{FLEX_HOME};

use Data::Dumper;
use File::Path qw(make_path remove_tree);
use File::Spec::Functions qw(curdir rel2abs abs2rel catdir catpath splitdir);
use Getopt::Long;
use Term::ANSIColor qw(:constants);
use Time::HiRes qw(time);

BEGIN {
	my @dirs = splitdir($0);
	push(@INC, catdir(@dirs[0 .. scalar(@dirs) - 2]));
}
END {
	
}

use constant {
	SYS_UNIX => 0,
	SYS_LINUX => 1,
	SYS_MAC => 2,
	SYS_WINDOWS => 3,
};

use constant {
	MODE_COMPILE_ALL => 0,
	MODE_COMPILE_DEFINITION => 1,
	MODE_COMPILE_ALL_WITH_DEFINITION => 2,
};

my $COMPILE_MODE = undef;

eval {require definition} or $COMPILE_MODE = MODE_COMPILE_ALL;

use constant {
	TYPE_PNG => 1,
	TYPE_JPG => 2,
	TYPE_GIF => 3,
	TYPE_BMP => 4,
	TYPE_WAV => 5,
	TYPE_MP3 => 6,
	TYPE_OGG => 7,
	TYPE_SWF => 8,
	TYPE_XML => 9,
	TYPE_TXT => 10,
	TYPE_JSON => 11,
};

use constant {
	TYPE_BITMAP => 0,
	TYPE_SPRITE => 1,
	TYPE_MOVIECLIP => 2,
};

use constant {
	CLASS_SPRITE => "Sprite",
	CLASS_MOVIECLIP => "MovieClip",
	CLASS_EXTENDED_MOVIECLIP => "ExtendedMovieClip",
	CLASS_EXTENDED_SPRITE => "ExtendedSprite",
	CLASS_BITMAPDATA => "BitmapData",
	CLASS_BITMAP => "Bitmap",
	CLASS_SOUND => "Sound",
	CLASS_BYTEARRAY => "ByteArray",
};

use constant {
	NAME => "name",
	PATH => "path",
	HAS_PROPS => "props",
	PROP_X => "x",
	PROP_Y => "y",
	PROP_A => "alpha",
	PROP_V => "visible",
};

use constant SYSTEM => $^O eq "MSWin32" ? SYS_WINDOWS : SYS_UNIX;
use constant BASE => "[base]";

my $libpath;
my $swc = 0;
my $debug = 0;
my $quality;
my $player;
my $verbose = 1;
my $swfoutput;
my $input = parse_options(\@ARGV);
my $tempdir = catdir(curdir(), "temp");
my $outdir = catdir(curdir(), "");
my $basedir = determine_base_directory();
my $xmlpath;

prompt_input() if ($input eq "");

print STDERR "Using ", setbold(BASE), " directory: $basedir\n";
print STDERR "Flex home in location: " . $ENV{FLEX_HOME} . "\n";

my $DEF = undef;
if (!defined($COMPILE_MODE) || $COMPILE_MODE != MODE_COMPILE_ALL) {
	my $xname = "definition.xml";
	my $absxmlpath = catpath("", catdir($basedir, $input), $xname);
	$xmlpath = catpath("", $input, $xname);
	if (-e $absxmlpath) {
		print STDERR "Load definition ", BASE, "$xmlpath\n";
		$DEF = Definition->new(data=>$absxmlpath);
		$DEF->load();
	} elsif (defined($COMPILE_MODE) && $COMPILE_MODE != MODE_COMPILE_ALL) {
		dieonerror("cannot use mode $COMPILE_MODE without a definition file in \'" . BASE . $xmlpath . "\'");
	}
	$COMPILE_MODE = defined($DEF) ? $DEF->getmode() : MODE_COMPILE_ALL if !defined($COMPILE_MODE);
}

my $width = 640;
my $height = 480;
my $bg = "#000000";
my $fps = 24;

if ($DEF) {
	$width = $DEF->getwidth();
	$height = $DEF->getheight();
	$bg = $DEF->getbackground();
	$fps = $DEF->getfps();
	$quality = $DEF->getquality() if !defined($quality);
} else {
	$quality = 80 if !defined($quality);
}

my $maincontent = "\t[SWF(width = \"$width\", height = \"$height\", backgroundColor = \"$bg\", frameRate = \"$fps\")]\n\n";
my $assetcount = 0;
my @assetfiles = ();
my $withmc = 0;
my $withsprite = 0;

createswf($input);

print STDERR "Delete $tempdir...\n";

foreach (@assetfiles) {
	unlink($_);
}

remove_tree($tempdir);

exit 0;


sub parse_options
{
	my ($params) = @_;

	my $help = 0;
	my $result = GetOptions (
		"debug!" => \$debug,
		"verbose+" => \$verbose,
		"mode=i" => \$COMPILE_MODE,
		"output=s" => \$swfoutput,
		"help!" => \$help,
		"quality=i" => \$quality,
		"player=f" => \$player,
		"swc!" => \$swc,
	);

	my $invalid = defined($COMPILE_MODE) && ($COMPILE_MODE > 2 || $COMPILE_MODE < 0);
	dieonerror("invalid mode $COMPILE_MODE") if $invalid;

	if ($help) {system("perldoc $0"); exit;}
	if (!$result || $invalid) {printmoreinfo(); exit;}

	if ($debug) {
		print STDERR "Include paths:\n";
		foreach my $inc (@INC) {
			print STDERR " ==> $inc\n";
		}
		print STDERR "\n";
	}

	return @$params[0] ? @$params[0] : "";
}

sub determine_base_directory
{
	my $base = undef;
	if (-d $input) {
		my $abspwd = rel2abs($input);
		my @dirs = splitdir($abspwd);
		$input = $dirs[-1] or die "Cannot use root directory\n";
		pop(@dirs);
		$base = catdir(@dirs);
	} elsif ($input eq "" || scalar(splitdir($input)) == 1) {
		my $defaultbase = $ENV{CREATESWF_BASE_DIR};
		$defaultbase =~ s/^~/$ENV{HOME}/ if $defaultbase;
		$base = $defaultbase ? catdir(splitdir($defaultbase)) : curdir();
	} else {
		print STDERR "$input does not exist\n";
		exit 0;
	}
	return $base;
}

sub prompt_input
{
	my $prompt = $ENV{CREATESWF_BASE_DIR} && -d $basedir;
	print STDERR "usage: $0 [options] directory\n";
	print STDERR " directory = path containing files to compile\n";
	print STDERR printmoreinfo() if (!$prompt);
	print STDERR " directory = or shortcut folder name containing assets to compile in $basedir\n\n" if ($prompt);
	exit if (!$prompt);
	opendir(CHOICEDIR, $basedir) or die "Could not open base directory $basedir\n";
	my @choices = readdir(CHOICEDIR);
	closedir(CHOICEDIR);
	my @realchoices = ();
	my $cnt = 0;
	foreach my $choice (@choices) {
		next if ($choice =~ /^\./);
		print STDERR " [ ", setbold(++$cnt), " ] $choice\n";
		push(@realchoices, $choice);
	}
	if (scalar(@realchoices) == 0) {
		print STDERR "No directories found in $basedir\n";
		exit;
	}
	unshift(@realchoices, 0);
	while (1) {
		print STDERR "\a\nEnter folder name or choice number: ";
		print STDERR BOLD if SYSTEM != SYS_WINDOWS;
		chomp ($input = <STDIN>);
		print STDERR RESET if SYSTEM != SYS_WINDOWS;
		next if ($input eq "" || substr($input, 0, 1) eq " ");
		$input = $realchoices[$input] if (isnumber($input) && $input < scalar(@realchoices));
		last;
	}
}

sub createswf
{
	my ($source) = @_;
	my $mainclass = "main";
	my $output = catpath("", $tempdir, $mainclass . ".as");
	my $name = defined($DEF) ? $DEF->getname() : undef;
	my $outswf = catpath("", $outdir , $swfoutput ? $swfoutput : $name ? $name : $source . ($swc ? ".swc" : ".swf"));
	my $currtime = time();

	make_path($tempdir, {error => \my $err});
	if (@$err) {
		for my $diag (@$err) {
			my ($file, $message) = %$diag;
			if ($file eq '') {
				print STDERR "General error: $message\n";
			} else {
				print STDERR "Problem unlinking $file: $message\n";
			}
		}
		die "Could not create $tempdir\n";
	} else {
		print STDERR "Write to $tempdir\n";
	}

	$maincontent .= "\tpublic class $mainclass extends " . &CLASS_SPRITE . "\n";
	$maincontent .= "\t{\n";

	if ($COMPILE_MODE == MODE_COMPILE_DEFINITION) {
		parse_definition();
	} else {
		my $srcdirpath = catdir($basedir, $source);
		print STDERR "Open directory $srcdirpath\n";
		opendir(BASEDIR, $srcdirpath) or die $! . "\n";
		my @files = readdir(BASEDIR);
		parse_directory(\@files, catdir($basedir, $source), $source);
		closedir(BASEDIR);
	}

	$currtime = getelapsedtime($currtime);
	print STDERR "Created ", setbold($assetcount), " asset files in ", setbold($currtime . " sec"), "\n";

	if ($withsprite) {
		$maincontent .= addcompilelist(++$assetcount, &CLASS_EXTENDED_SPRITE);
		my $extsprite = catpath("", $tempdir, &CLASS_EXTENDED_SPRITE . ".as");
		print STDERR "Create class $extsprite \n";
		open(ASFILE, ">" . $extsprite) or die "Cannot create file $extsprite: $!";
		print ASFILE createclass_extsprite();
		close(ASFILE);
	}

	if ($withmc) {
		$maincontent .= addcompilelist(++$assetcount, &CLASS_EXTENDED_MOVIECLIP);
		my $extmc = catpath("", $tempdir, &CLASS_EXTENDED_MOVIECLIP . ".as");
		print STDERR "Create class $extmc \n";
		open(ASFILE, ">" . $extmc) or die "Cannot create file $extmc: $!";
		print ASFILE createclass_extmc();
		close(ASFILE);
	}

	$maincontent .= "\n";
	$maincontent .= "\t\tpublic function $mainclass () {\n";
	$maincontent .= "\t\t\tsuper ();\n";

	print STDERR "Create main class $output\n";
	open(ASFILE, ">" . $output) or die "Cannot create file $output: $!";
	print ASFILE create_class($output, &CLASS_SPRITE, $maincontent, "\timport flash.display." . &CLASS_SPRITE . ";\n\n");
	close(ASFILE);

	if ($debug == 1) {
		print STDERR "Debug mode exits here\n";
		exit;
	}

	my $flxlib = catdir($ENV{FLEX_HOME}, qw(frameworks libs));
	my $playerdir = catdir($flxlib, "player");
	my $flashversion = defined($player) ? $player: (defined($DEF) ? $DEF->getplayer() : 10);
	my $useplayerdir = catdir($playerdir, $flashversion);

	if (!defined($player) && (!defined($flashversion) || (! -d $useplayerdir))) {
		opendir(PDIR, $playerdir) or die "no flex player directory $playerdir\n";
		my @dirs = readdir(PDIR);
		$flashversion = (sort { $b cmp $a } @dirs)[0];
		$useplayerdir = catdir($playerdir, $flashversion);
		closedir(PDIR);
	}

	print STDERR "Compile $output as ", setbold($outswf), " ...\n";
	$currtime = time();

	my $compile;
	if ($swc) {
		$compile = "compc ";
		$compile .= " -source-path " . $tempdir;
		$compile .= " -include-classes " . $mainclass;
	} else {
		$compile = "mxmlc " . $output;
		$compile .= " -default-size=" . $width . "," . $height;
		$compile .= " -default-background-color=0x" . substr($bg, index($bg, "#") + 1);
		$compile .= " -static-link-runtime-shared-libraries=true";
	}

	$compile .= " -output " . $outswf;
	$compile .= " -target-player=$flashversion";
	$compile .= " -strict";
	$compile .= " -library-path+=" . catpath("", $useplayerdir, "playerglobal.swc");
	$compile .= " -library-path+=" . catpath("", $flxlib, "core.swc -use-network=true");
	$compile .= " -library-path+=" . catdir("", splitdir($libpath));
	$compile .= " -define=CONFIG::DEBUG,true";
	$compile .= " -define=CONFIG::FP10,true";
	$compile .= " -define=CONFIG::FP9,false";

	my $success = system($compile) == 0 or print STDERR "\n >>> Compilation aborted... \n\n";
	if($success) {
		my $size = int((-s $outswf) / (1024 * 1024) * 100) / 100;
		$currtime = getelapsedtime($currtime);
		print STDERR "Compilation of ", setbold($size . " MB"), " took ", setbold($currtime . " sec"), "\n" if $success;
	}
	print STDERR "Delete " . $output . "...\n";
	unlink($output);
}

sub parse_directory
{
	my ($files, $srcdir, $source) = @_;

	foreach (@$files) {
		next if ($_ =~ /^\./);
		my $subdir = catdir($srcdir, $_);
		if (-d $subdir) {
			opendir(DIR, $subdir) or die $!;
			my @nested = readdir(DIR);
			closedir(DIR);
			parse_directory(\@nested, $subdir, catdir($source, $_));
			next;
		}
		my $type = getfiletype($_);
		next if ($type == 0);

		my %bitmaps;
		my $endidx = index($_, "___");
		my $classname = substr($_, 0, $endidx > -1 ? $endidx : rindex($_, "."));
		my $ctype = TYPE_BITMAP;
		next if !parse_indexed_sprite($files, $srcdir, $_, \$classname, \%bitmaps, \$ctype);

		my $assetfile = catpath("", $tempdir, $classname . ".as");
		$maincontent .= addcompilelist(++$assetcount, $classname);
		open(ASASSETFILE, ">" . $assetfile) or die "Cannot create file $assetfile: $!";

		my $content;
		if ($type == TYPE_PNG || $type == TYPE_JPG || $type == TYPE_GIF || $type == TYPE_BMP) {
			$content = createclass_movieclip($classname, \%bitmaps) if ($ctype == TYPE_MOVIECLIP);
			$content = createclass_sprite($classname, \%bitmaps) if ($ctype == TYPE_SPRITE);
			$content = createclass_bitmap($classname, catpath("", $srcdir, $_)) if ($ctype == TYPE_BITMAP);
		} elsif ($type == TYPE_MP3 || $type == TYPE_WAV || $type == TYPE_OGG) {
			$content = createclass_sound($classname, catpath("", $srcdir, $_));
		} elsif ($type == TYPE_SWF || $type == TYPE_TXT || $type == TYPE_JSON || $type == TYPE_BMP || $type == TYPE_XML) {
			$content = createclass_octetstream($classname, catpath("", $srcdir, $_));
		} else {
			print STDERR "warning: unrecognized file type $_ will extend \'" . CLASS_BYTEARRAY . "\'\n";
			$content = createclass_octetstream($classname, catpath("", $srcdir, $_));
		}

		print ASASSETFILE $content or die "write failed in " . (caller(0))[3] . "\n";
		close(ASASSETFILE);
		push(@assetfiles, $assetfile);
	}
}

sub parse_indexed_sprite
{
	my ($files, $srcdir, $path, $nameref, $bitmaps, $type) = @_;

	my $classname = $$nameref;
	my $ismc = ($classname =~ m/^mc[0-9]+__/);
	my $ismc0 = ($classname =~ m/^mc0__/);
	$$type = TYPE_MOVIECLIP if $ismc0;
	return 0 if ($ismc && !$ismc0);

	my $isspr = ($classname =~ m/^sp[0-9]+__/);
	my $isspr0 = ($classname =~ m/^sp0__/);
	$$type = TYPE_SPRITE if $isspr0;
	return 0 if ($isspr && !$isspr0);

	$withmc = 1 if $ismc0;
	$withsprite = 1 if $isspr0;
	$classname =~ s/^mc0__// if $ismc0;
	$classname =~ s/^sp0__// if $isspr0;

	my $prefix = ($ismc ? "mc" : "sp");

	if ($ismc0 || $isspr0) {
		my $fx = "__";
		foreach my $file (@$files) {
			next if (index($file, $classname) != index($file, $fx) + 2);
			my $idx = substr($file, length($prefix), index($file, "_") - 2);
			my $pth = catpath("", $srcdir, $file);
			my $relative = abs2rel($pth);
			my %elem = ();
			my $frame;
			if ($COMPILE_MODE == MODE_COMPILE_ALL_WITH_DEFINITION) {
				my $namenoext = substr($file, 0, rindex($file, "."));
				$frame = $DEF->getframe($classname, $idx, $ismc);
				$frame->{&HAS_PROPS} = 1 if $frame;
			}
			$frame->{&PATH} = $relative;
			$frame->{&NAME} = $classname;
			$bitmaps->{$idx} = $frame;
		}
	}
	$$nameref = $classname;
	return 1;
}

sub parse_definition
{
	my $mcs = $DEF->getmovieclips();
	my $count = $assetcount;
	create_definition_sprites($mcs, "mc") if defined($mcs);
	$withmc = ($assetcount > $count);

	my $spr = $DEF->getspriteclips();
	$count = $assetcount;
	create_definition_sprites($spr, "spr") if defined($spr);
	$withsprite = ($assetcount > $count);

	my $bitmaps = $DEF->getbitmaps();
	create_definition_objects($bitmaps, "bmp", \&createclass_bitmap) if defined($bitmaps);

	my $sounds = $DEF->getsounds();
	create_definition_objects($sounds, "snd", \&createclass_sound) if defined($sounds);

	my $binaries = $DEF->getbinaries();
	create_definition_objects($binaries, "bin", \&createclass_octetstream) if defined($binaries);
}

sub create_definition_sprites {
	my ($clips, $type) = @_;
	my $ismc = $type eq "mc";
	dieonerror("one of the \'$type\' nodes has no \'class\' attribute!") if $clips && ref $clips ne "HASH";
	foreach my $classname (keys %$clips) {
		my $sprite = $clips->{$classname};
		my %bitmaps = ();
		my $basepath = defined($sprite->{&PATH}) ? $sprite->{&PATH} : "";
		my $frames = $ismc ? $DEF->getframes($classname) : $DEF->getobjects($classname);
		if (defined($frames)) {
			my $cnt = 0;
			foreach my $frame (@$frames) {
				my $nname = $frame->{&NAME};
				my $path = checkpath($nname, $basepath, $frame->{&PATH});
				$frame = $DEF->checkvalues($frame);
				$frame->{&HAS_PROPS} = 1;
				$frame->{&PATH} = abs2rel($path);
				$bitmaps{$cnt++} = $frame;
			}
		} elsif (defined($sprite->{&PATH})) {
			my %frm = ();
			my $frame = $DEF->copyvalues($sprite, \%frm);
			$frame->{&HAS_PROPS} = 1;
			$frame->{&PATH} = abs2rel(checkpath($sprite->{&NAME}, "", $basepath));
			$frame = $DEF->checkvalues($frame);
			$DEF->resetvalues($sprite);
			$bitmaps{0} = $frame;
		}
		my $assetfile = catpath("", $tempdir, $classname . ".as");
		$maincontent .= addcompilelist(++$assetcount, $classname);
		open(ASASSETFILE, ">" . $assetfile) or die "Cannot create file $assetfile: $!";
		my $content = $ismc ? createclass_movieclip($classname, \%bitmaps) : createclass_sprite($classname, \%bitmaps);
		print ASASSETFILE $content or die "write failed in " . (caller(0))[3] . "\n";
		close(ASASSETFILE);
	}
}

sub create_definition_objects {
	my ($objects, $type, $function) = @_;
	my $ismc = $type eq "mc";
	dieonerror("one of the \'$type\' nodes has no \'class\' attribute!") if $objects && ref $objects ne "HASH";
	foreach my $classname (keys %$objects) {
		my $object = $objects->{$classname};
		$object->{&HAS_PROPS} = 1;
		$object = $DEF->checkvalues($object);
		my $assetfile = catpath("", $tempdir, $classname . ".as");
		$maincontent .= addcompilelist(++$assetcount, $classname);
		open(ASASSETFILE, ">" . $assetfile) or die "Cannot create file $assetfile: $!";
		my $content = &$function($classname, checkpath($object->{&NAME}, "", $object->{&PATH}));
		print ASASSETFILE $content or die "write failed in" . (caller(0))[3] . "\n";
		close(ASASSETFILE);
	}
}

sub checkpath {
	my ($nname, $basepath, $path) = @_;
	$path =~ s/^\s+// if $path;
	$nname = "" if !defined $nname;
	dieonerror("no path specified for node \'$nname\' in \'" . BASE . $xmlpath . "\'") if !defined($path) || $path eq "";
	$path = catdir($basepath, $path);
	my @dirs = splitdir($path);
	my $file = $dirs[-1];
	$path = catdir($input, $path);
	my $abspath = catpath("", catdir($basedir, $input, @dirs[0 .. scalar(@dirs) - 2]), $file);
	$nname = "" if !defined($nname);
	dieonerror("path \'" . BASE . "$path\' does not exist for node \'$nname\' in \'" . BASE . $xmlpath . "\'") if (! -e $abspath);
	dieonerror("invalid path \'", BASE, "$path\' for node \'$nname\' in \'" . $xmlpath . "\'") if !get_ext($path);
	return $abspath;
}

sub getfiletype
{
	my ($name) = @_;
	my $ext = get_ext($name);
	$ext = "\L$ext";
	return TYPE_PNG if ($ext eq "png");
	return TYPE_JPG if ($ext eq "jpg");
	return TYPE_GIF if ($ext eq "gif");
	return TYPE_BMP if ($ext eq "bmp");
	return TYPE_WAV if ($ext eq "wav");
	return TYPE_MP3 if ($ext eq "mp3");
	return TYPE_OGG if ($ext eq "ogg");
	return TYPE_SWF if ($ext eq "swf");
	return TYPE_XML if ($ext eq "xml");
	return TYPE_TXT if ($ext eq "txt");
	return TYPE_JSON if ($ext eq "json");
	return 0;
}

sub get_ext {
	my ($file) = @_;
	my $idx = rindex($file, ".");
	print STDERR "Warning: Invalid file type in \'$file\'\n" if ($idx == -1);
	return undef if ($idx == -1);
	my $ext = substr($file, $idx + 1);
	return $ext;
}

sub get_mime
{
	my ($name) = @_;
	my $ext = get_ext($name);
	my $type = getfiletype($name);
	return ("image/" . $ext) if ($type == TYPE_PNG || $type == TYPE_JPG || $type == TYPE_GIF);
	return ("audio/mpeg") if ($type == TYPE_MP3);
	return ("application/octet-stream");
}

sub isnumber
{
    shift =~ /^-?\d+\.?\d*$/;
}

sub printmoreinfo
{
	print STDERR "\nsee \'perldoc $0\' for more info\n";
}

sub addcompilelist
{
	my ($count, $classname) = @_;
	return "\t\tpublic var force_compile_" . $count . ":" . $classname . ";\n";
}

sub dieonerror
{
	my ($str) = @_;
	my $err = "\nError: ";
	if (SYSTEM != SYS_WINDOWS) {
		$err = BOLD RED . $err . RESET;
	}
	die $err . $str, "\n\n";
}

sub setbold {
	my ($str) = @_;
	return SYSTEM != SYS_WINDOWS ? BOLD . $str . RESET : $str;
}

sub getelapsedtime {
	my ($prevtime) = @_;
	return int((time() - $prevtime) * 1000) / 1000;
}

sub createclass_octetstream
{
	my ($name, $source) = @_;
	my $mime = get_mime($source);
	my $imports = "\timport flash.utils." . &CLASS_BYTEARRAY . ";\n\n";
	$source = abs2rel($source);
	$source =~ s/\\/\//g;
	$source = "../" . $source;
	my $content = "\t[Embed(source=\"$source\", mimeType=\"$mime\")]\n";
	$content .= "\tpublic class $name extends " . &CLASS_BYTEARRAY . " {\n";
	$content .= "\t\tpublic function $name () {\n";
	$content .= "\t\t\tsuper ();\n";

	create_class($name, &CLASS_BYTEARRAY, $content, $imports);
}

sub createclass_sound
{
	my ($name, $source) = @_;
	my $type = getfiletype($source);
	return createclass_octetstream($name, $source) if ($type != TYPE_MP3);
	my $mime = get_mime($source);
	my $imports = "\timport flash.media." . &CLASS_SOUND . ";\n";
	$imports .= "\timport flash.media.SoundLoaderContext;\n";
	$imports .= "\timport flash.net.URLRequest;\n\n";
	$source = abs2rel($source);
	$source =~ s/\\/\//g;
	$source = "../" . $source;
	my $content = "\t[Embed(source=\"$source\", mimeType=\"$mime\")]\n";
	$content .= "\tpublic class $name extends " . &CLASS_SOUND . " {\n";
	$content .= "\t\tpublic function $name (stream:URLRequest = null, context:SoundLoaderContext = null) {\n";
	$content .= "\t\t\tsuper (stream, context);\n";

	create_class($name, &CLASS_SOUND, $content, $imports);
}

sub createclass_bitmap
{
	my ($name, $source) = @_;

	my $mime = get_mime($source);
	my $type = getfiletype($source);
	return createclass_octetstream($name, $source) if ($type == TYPE_BMP);
	my $imports = "\timport flash.display." . &CLASS_BITMAPDATA . ";\n\n";
	my $usedef = ($COMPILE_MODE == MODE_COMPILE_ALL_WITH_DEFINITION || $COMPILE_MODE == MODE_COMPILE_DEFINITION);
	my $bmp = ($usedef && $DEF) ? $DEF->getbitmap($name) : undef;

	$source = abs2rel($source);
	$source =~ s/\\/\//g;
	$source = "../" . $source;

	my $qualityattr = ($type == TYPE_GIF) ? ", compression=\"true\", quality=\"$quality\"" : "";
	my $content = "\t[Embed(source=\"$source\", mimeType=\"$mime\"$qualityattr)]\n";
	$content .= "\tpublic class $name extends " . &CLASS_BITMAPDATA . " {\n";
	$content .= "\t\tpublic function $name (width:int, height:int, transparent:Boolean = true, fillColor:uint = 0xFFFFFFFF) {\n";
	$content .= "\t\t\tsuper (width, height, transparent, fillColor);\n";

	create_class($name, &CLASS_BITMAPDATA, $content, $imports);
}

sub createclass_movieclip
{
	my ($name, $bitmaps) = @_;

	my $imports .= "\timport flash.display." . &CLASS_BITMAP . ";\n\n";
	my $content = "\tpublic class $name extends " . &CLASS_EXTENDED_MOVIECLIP . " {\n";
	my $usedef = ($COMPILE_MODE == MODE_COMPILE_ALL_WITH_DEFINITION || $COMPILE_MODE == MODE_COMPILE_DEFINITION);
	my $mc = ($usedef && $DEF) ? $DEF->getmovieclip($name) : undef;
	my $bmpcount = 0;

	$content .= create_sprite_content($name, $bitmaps, 1, \$bmpcount);
	$content .= setproperties($mc) if $mc;

	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function get totalFrames ():int {\n";
	$content .= "\t\t\treturn $bmpcount\n";

	create_class($name, &CLASS_EXTENDED_MOVIECLIP, $content, $imports);
}

sub createclass_sprite
{
	my ($name, $bitmaps) = @_;

	my $imports .= "\timport flash.display." . &CLASS_BITMAP . ";\n\n";
	my $content = "\tpublic class $name extends " . &CLASS_EXTENDED_SPRITE . " {\n";
	my $usedef = ($COMPILE_MODE == MODE_COMPILE_ALL_WITH_DEFINITION || $COMPILE_MODE == MODE_COMPILE_DEFINITION);
	my $sp = ($usedef && $DEF) ? $DEF->getsprite($name) : undef;
	my $bmpcount = 0;

	$content .= create_sprite_content($name, $bitmaps, 0);
	$content .= setproperties($sp) if $sp;

	create_class($name, &CLASS_EXTENDED_SPRITE, $content, $imports);
}

sub setproperties {
	my ($obj) = @_;
	my $content = "";
	$content .= "\t\t\tthis.x = " . $obj->{&PROP_X} . ";\n" if ($obj->{&PROP_X} != 0);
	$content .= "\t\t\tthis.y = " . $obj->{&PROP_Y} . ";\n" if ($obj->{&PROP_Y} != 0);
	$content .= "\t\t\tthis.alpha = " . $obj->{&PROP_A} . ";\n" if ($obj->{&PROP_A} < 1);
	$content .= "\t\t\tthis.visible = false;\n" if ($obj->{&PROP_V} == 0);
	return $content;
}

sub create_sprite_content
{
	my ($name, $bmphash, $ismc, $numbitmaps) = @_;

	my $content = "";
	my $bmpcount = 0;
	my $bmpname = "bmpref";
	my $ctorcontent = "\n";

	$ctorcontent .= "\t\tpublic function $name () {\n";
	$ctorcontent .= "\t\t\tsuper ();\n";

	foreach my $key (sort { $a <=> $b } keys(%$bmphash)) {
		my $subhash = $bmphash->{$key};
		my $path = $subhash->{&PATH};
		my $mime = get_mime($path);
		my $type = getfiletype($path);
		my $bname = $bmpname . $bmpcount;
		my $qualityattr = ($type == TYPE_GIF) ? ", compression=\"true\", quality=\"$quality\"" : "";

		$path =~ s/\\/\//g;
		$path = "../" . $path;
		$content .= "\t\t[Embed(source=\"" . $path . "\", mimeType=\"$mime\"$qualityattr)] private var $bname:Class;\n";

		if ($subhash->{&HAS_PROPS}) {
			my $file = (splitdir($path))[-1];
			my $didx = rindex($file, ".");
			$file = substr($file, 0, $didx) if ($didx != -1);
			$ctorcontent .= "\t\t\taddObject(new $bname() as Bitmap";

			if ($subhash) {
				my $rest = ($subhash->{&PROP_V} == 0);
				my $params = "";
				$params .= ", false" if ($rest);
				$params = ", " . ($subhash->{&PROP_A} . $params) if ($rest ||= $subhash->{&PROP_A} < 1);
				$params = ", " . ($subhash->{&PROP_Y} . $params) if ($rest ||= $subhash->{&PROP_Y} != 0);
				$params = ", " . ($subhash->{&PROP_X} . $params) if ($rest ||= $subhash->{&PROP_X} != 0);
				$ctorcontent .= $params;
			}
			$ctorcontent .= ");\n";
		} else {
			$ctorcontent .= "\t\t\t" . ($ismc ? "addObject" : "addChild") . "(new $bname() as Bitmap);\n";
		}

		++$bmpcount;
	}

	$$numbitmaps = $bmpcount if defined($numbitmaps);
	$content .= $ctorcontent;
}

sub createclass_extmc
{
	my $imports .= "\timport flash.display." . &CLASS_BITMAP . ";\n";
	$imports .= "\timport flash.display.DisplayObject;\n";
	$imports .= "\timport flash.display." . &CLASS_MOVIECLIP . ";\n\n";

	my $content = "\tpublic class " . &CLASS_EXTENDED_MOVIECLIP . " extends " . &CLASS_MOVIECLIP . " {\n";
	$content .= "\t\tprivate var _frames:Vector.<DisplayObject> = new Vector.<DisplayObject>();\n";
	$content .= "\t\tprivate var _frame:int = 1;\n\n";
	$content .= "\t\tpublic function " . &CLASS_EXTENDED_MOVIECLIP . " () {\n";
	$content .= "\t\t\tsuper ();\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function gotoAndPlay (frame:Object, scene:String = null):void {\n";
	$content .= "\t\t\tshowframe(int(frame))\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function gotoAndStop (frame:Object, scene:String = null):void {\n";
	$content .= "\t\t\tshowframe(int(frame))\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function nextFrame ():void {\n";
	$content .= "\t\t\tshowframe(int(_frame + 1))\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function prevFrame ():void {\n";
	$content .= "\t\t\tshowframe(int(_frame - 1))\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tprotected function addObject (object:DisplayObject, x:int = 0, y:int = 0, alpha:Number = 1.0, visible:Boolean = true):void {\n";
	$content .= "\t\t\tobject.x = x;\n";
	$content .= "\t\t\tobject.y = y;\n";
	$content .= "\t\t\tobject.alpha = alpha;\n";
	$content .= "\t\t\tobject.visible = visible;\n";
	$content .= "\t\t\t_frames.push(addChild(object));\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tprotected function showframe (frame:int):void {\n";
	$content .= "\t\t\tconst numFrames:int = _frames.length;\n";
	$content .= "\t\t\tif (numFrames < 1) return;\n";
	$content .= "\t\t\telse if (frame > numFrames) frame = numFrames;\n";
	$content .= "\t\t\telse if (frame < 1) frame = 1;\n";
	$content .= "\t\t\t_frame = frame--;\n";
	$content .= "\t\t\tfor (var n:int = 0; n < numFrames; ++n) {\n";
	$content .= "\t\t\t\t_frames[n].visible = false;\n";
	$content .= "\t\t\t}\n";
	$content .= "\t\t\t_frames[frame].visible = true;\n";
	$content .= "\t\t}\n\n";
	$content .= "\t\tpublic override function get currentFrame ():int {\n";
	$content .= "\t\t\treturn _frame\n";

	create_class(&CLASS_EXTENDED_MOVIECLIP, &CLASS_MOVIECLIP, $content, $imports);
}

sub createclass_extsprite
{
	my ($name, $bitmaps) = @_;
	my $imports .= "\timport flash.display." . &CLASS_BITMAP . ";\n";
	$imports .= "\timport flash.display.DisplayObject;\n";
	$imports .= "\timport flash.display." . &CLASS_SPRITE . ";\n\n";

	my $content = "\tpublic class " . &CLASS_EXTENDED_SPRITE . " extends " . &CLASS_SPRITE . " {\n";
	$content .= "\n";
	$content .= "\t\tpublic function " . &CLASS_EXTENDED_SPRITE . " () {\n";
	$content .= "\t\t\tsuper ();\n";
	$content .= "\t\t}\n\n";

	if ($COMPILE_MODE == MODE_COMPILE_DEFINITION || $COMPILE_MODE == MODE_COMPILE_ALL_WITH_DEFINITION) {
		$content .= "\t\tprotected function addObject (object:DisplayObject, x:int = 0, y:int = 0, alpha:Number = 1.0, visible:Boolean = true):void {\n";
		$content .= "\t\t\tobject.x = x;\n";
		$content .= "\t\t\tobject.y = y;\n";
		$content .= "\t\t\tobject.alpha = alpha;\n";
		$content .= "\t\t\tobject.visible = visible;\n";
		$content .= "\t\t\taddChild(object);\n";
		$content .= "\t\t}\n\n";
	}

	$content .= "\t\tpublic function center():void {\n";
	$content .= "\t\t\tfor (var n:int = 0; n < numChildren; ++n) {\n";
	$content .= "\t\t\t\tvar obj:DisplayObject = getChildAt(n);\n";
	$content .= "\t\t\t\tobj.x = -obj.width * 0.5;\n";
	$content .= "\t\t\t\tobj.y = -obj.height * 0.5;\n";
	$content .= "\t\t\t}\n";

	create_class(&CLASS_EXTENDED_SPRITE, &CLASS_SPRITE, $content, $imports);
}

sub create_class
{
	my ($name, $baseclass, $content, $imports) = @_;
	my $class = "package\n{\n";
	print STDERR "Creating class \'$name\' extends \'$baseclass\'\n" if $verbose > 1;
	$class .= $imports;
	$class .= "\t/**\n";
	$class .= "\t * \@author: $0\n";
	$class .= "\t * \@description: auto-generated file by $0\n";
	$class .= "\t */\n\n";
	$class .= $content;
	$class .= "\t\t}\n";
	$class .= "\t}\n";
	$class .= "}\n";
}

__END__

=head1 NAME

createswf - script to compile a SWF file from a set of files

=head1 SYNOPSIS

createswf [options] directory

=head1 DESCRIPTION

B<createswf> will scan the given directory recursively and create a corresponding temporary ".as" file for each valid file. It'll then compile them into a SWF.

The program uses one mandatory environment variable B<FLEX_HOME> containing the path to the Flex SDK. It also uses on optional environment variable B<CREATESWF_BASE_DIR> that points to a default base directory containing folders to compile for shortcut use.

The following command compiles a SWF from all files residing in directory "assets/images" which is relative to the current directory:

=over 8

createswf assets/images/

=back

=head1 OPTIONS

=over 8

=item B<-d>, B<--debug>

Leave auto-generated files in ./temp folder and output more debug info

=item B<-m>, B<--mode>

Set parsing-compilation mode. Default is --mode=0

  0 => Recursively parse directory & compile all valid files
  1 => Parse definition.xml in folder target and compile it
  2 => Do mode 0 with the addition of using definition.xml

=item B<-o>, B<--output>

Set SWF output name

=item B<-?>, B<-h>, B<--help>

Prints a brief help message and exits.

=item B<-p>, B<--player>

Set flash player version to compile for

=item B<-q>, B<--quality>

Set JPEG quality of embedded image data

=item B<-s>, B<--swc>

Set output as .swc format instead of default .swf format

=item B<-v>, B<--verbose>

Increase verbosity level & print more information during execution

=back

=head1 AUTHOR

Harry Kunz = harry.kunz@ymail.com

=cut

