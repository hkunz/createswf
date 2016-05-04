#!/usr/bin/perl
#
# definition.pm
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

# hkunz = harry.kunz@ymail.com 14/07/2012
# Module used by createswf.pl to get definitions of object properties in definition.xml
#

package Definition;

use strict;
use warnings;

use XML::SAX::Expat;
use XML::Simple;
use Data::Dumper;

my $_data;
my $_xmlpath;

sub new {
    my ($class, %args) = @_;
    return bless { %args }, $class;
}

sub load {
	my ($self) = @_;
	my $xml = new XML::Simple;
	$_xmlpath = $self->{"data"};
	$_data = $xml->XMLin($_xmlpath, ForceArray => 1, KeyAttr => ["class"]);
}

sub getxmlpath {
	my $self = shift;
	return $_xmlpath;
}

sub getplayer {
	my $self = shift;
	my $default = 11;
	my $node = $_data->{"player"};
	return $default if !$node;
	my $value = $node->[0];
	return $default if ref $value eq 'HASH';
	return $value;
}

sub getquality {
	my $self = shift;
	my $default = 80;
	my $node = $_data->{"quality"};
	return $default if !$node;
	my $value = $node->[0];
	return $default if ref $value eq 'HASH';
	return $value;
}

sub getwidth {
	my $self = shift;
	my $w = $_data->{"width"};
	return $w ? $w : 1024;
}

sub getheight {
	my $self = shift;
	my $h = $_data->{"height"};
	return $h ? $h : 768;
}

sub getfps {
	my $self = shift;
	my $fps = $_data->{"fps"};
	return $fps ? $fps : 36;
}

sub getbackground {
	my $self = shift;
	my $bg = $_data->{"background"};
	return $bg ? $bg : "#FFFFFF";
}

sub getmode {
	my $self = shift;
	my $node = $_data->{"mode"};
	return 0 if !$node;
	my $value = $node->[0];
	return 0 if ref $value eq 'HASH';
	return $value;
}

sub getname {
	my $self = shift;
	my $node = $_data->{"name"};
	return undef if !$node;
	my $value = $node->[0];
	return undef if ref $value eq 'HASH';
	return $value;
}

sub __getlib {
	my $self = shift;
	my $lib = $_data->{"library"};
	return undef if !$lib;
	return $lib->[0];
}

sub getmovieclips {
	my $self = shift;
	return __getclips($self, "movieclips", "mc");
}

sub getspriteclips {
	my $self = shift;
	return __getclips($self, "sprites", "spr");
}

sub getbitmaps {
	my $self = shift;
	return __getclips($self, "bitmaps", "bmp");
}

sub getsounds {
	my $self = shift;
	return __getclips($self, "sounds", "snd");
}

sub getbinaries {
	my $self = shift;
	return __getclips($self, "binaries", "bin");
}

sub __getclips {
	my $self = shift;
	my $nodename = shift;
	my $childnodename = shift;
	my $lib = __getlib($self);
	return undef if (!$lib);
	my $node = $lib->{$nodename};
	return undef if (!$node);
	my $elem = $node->[0];
	my $clips = $elem->{$childnodename};
	return $clips;
}

sub __getclip {
	my $self = shift;
	my $name = shift;
	my $parentnodename = shift;
	my $nodename = shift;
	my $lib = __getlib($self);
	return undef if (!$lib);
	my $node = $lib->{$parentnodename};
	return undef if (!$node);
	my $elem = $node->[0];
	my $clips = $elem->{$nodename};
	return undef if (!$clips);
	die "Error: node \'$nodename\' has no \'class\' attribute!\n" if ref $clips ne 'HASH';
	my $clip = $clips->{$name};
	$clip = checkvalues($self, $clip);
	return $clip;
}

sub getmovieclip {
	my $self = shift;
	my $name = shift;
	my $lib = __getlib($self);
	my $clip = __getclip($self, $name, "movieclips", "mc");
	$clip->{"fps"} = 12 if !defined($clip->{"fps"});
	return $clip;
}

sub getsprite {
	my $self = shift;
	my $name = shift;
	my $lib = __getlib($self);
	my $clip = __getclip($self, $name, "sprites", "spr");
	return $clip;
}

sub getbitmap {
	my $self = shift;
	my $name = shift;
	my $lib = __getlib($self);
	my $clip = __getclip($self, $name, "bitmaps", "bmp");
	return $clip;
}

#get mc frames of an mc by linkage name
sub getframes {
	my $self = shift;
	my $name = shift;
	my $mcs = getmovieclips($self);
	my $clip = $mcs->{$name};
	return undef if !defined($clip);
	my $frames = $clip->{"frame"};
	return $frames;
}

#get sprite objects of a sprite by linkage name
sub getobjects {
	my $self = shift;
	my $name = shift;
	my $clips = getspriteclips($self);
	my $clip = $clips->{$name};
	return undef if !defined($clip);
	my $frames = $clip->{"object"};
	return $frames;
}

#get mc frame if $ismc is true or sprite object if false
sub getframe {
	my $self = shift;
	my $name = shift;
	my $index = shift;
	my $ismc = shift;
	return __getobject($self, $name, $index, "movieclips", "mc", "frame") if $ismc;
	return __getobject($self, $name, $index, "sprites", "spr", "object");
}

sub __getobject {
	my $self = shift;
	my $clipname = shift;
	my $index = shift;
	my $groupkey = shift;
	my $clipkey = shift;
	my $childkey = shift;
	my $lib = __getlib($self);
	return undef if (!$lib);
	my $clips = $lib->{$groupkey};
	return undef if (!$clips);
	my $elem = $clips->[0];
	my $clipsnode = $elem->{$clipkey};
	return undef if (!$clipsnode);
	die "Error: node \'$clipkey\' has no \'class\' attribute!\n" if ref $clipsnode ne 'HASH';
	my $clip = $clipsnode->{$clipname};
	return undef if (!$clip);
	my $frmnode = $clip->{$childkey};
	if (!$frmnode) {
		$frmnode = checkvalues($self, copyvalues($self, $clip));
		resetvalues($self, $clip);
		return $frmnode;
	}
	my $frm = $frmnode->[$index];
	return checkvalues($self, $frm);
}

sub checkvalues {
	my $self = shift;
	my $obj = shift;
	my $v = $obj->{"visible"};
	my $a = $obj->{"alpha"};
	$obj->{"x"} = 0 if !defined($obj->{"x"});
	$obj->{"y"} = 0 if !defined($obj->{"y"});
	$obj->{"alpha"} = 1 if !defined($a) || $a > 1 || $a < 0;
	$obj->{"visible"} = defined($v) ? ($v eq "true" || $v eq "1" ? 1 : 0) : 1;
	return $obj;
}

sub copyvalues {
	my $self = shift;
	my $src = shift;
	my $dest = shift;
	$dest->{"x"} = $src->{"x"};
	$dest->{"y"} = $src->{"y"};
	$dest->{"alpha"} = $src->{"alpha"};
	$dest->{"visible"} = $src->{"visible"};
	return $dest;
}

sub resetvalues {
	my $self = shift;
	my $obj = shift;
	$obj->{"x"} = 0;
	$obj->{"y"} = 0;
	$obj->{"alpha"} = 1;
	$obj->{"visible"} = 1;
	return $obj;
}

1;
