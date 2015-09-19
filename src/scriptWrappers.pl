#!/usr/bin/perl

# Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

# scriptWrappers.pl scans all (resp. given) header files and creates necessary wrapper functions

# To make function accessible from python see script.h

#use warnings;
#use strict;
#use Data::Dumper;
use v5.10;

my $print_headers=0;
my @files;
for (@ARGV) {
	if ($_ eq "--headers"){
		$print_headers=1;
		next;
	}
	push @files, $_;
}
@ARGV=@files;

sub getType($) {
	local $_=shift;

	my $rx_ignore      = qr/\s*(?:\b(?:extern|const)\b\s*)*/;
	my $rx_type        = qr/(?:char\*|bool|int|float|void)/;
	my %mapping = (
		"char*" => {
			pyType   => "PyString",
			pyFormat => "s",
			converter=> "PyString_AsString"
		},
		bool => {
			pyType   => "PyInt",
			pyFormat => "i",
			converter=> "PyInt_AsLong"
		},
		int => {
			pyType   => "PyInt",
			pyFormat => "i",
			converter=> "PyInt_AsLong"
		},
		float => {
			pyType   => "PyFloat",
			pyFormat => "f",
			converter=> "PyFloat_AsDouble"
		},
		void => {
			pyFormat => ""
		}
	);

	s/\s*\*/\*/g;

	my ($type, $isArray) = /^$rx_ignore($rx_type)(\*)?$rx_ignore$/;
	return () unless defined $type;
	$isArray //= "";

	my %mapped = %{$mapping{$type}};
	return () unless %mapped;

	return (cType=>$type, isArray=>$isArray, %mapped);
}

my ($fc,$fh);
unless ($print_headers) {
	open $fc, ">", "scriptWrappers.c.tmp" or die "Cannot open scriptWrappers.c.tmp";
	open $fh, ">", "scriptWrappers.h.tmp" or die "Cannot open scriptWrappers.c.tmp";
	my $str= <<EOF;
// Generated by 'scriptWrappers.pl'
// vim: filetype=c
EOF
	say $fc $str;
	say $fh $str;
}


my $lastFile="";
my $comment=0;
my $last=0;
my $wrappersList="";
my %functions;
main:
while (<>) {
	say STDERR "Skipping: $last" if $last;
	$last=0;

	chomp;
	/\[SCRIPT_NAME:\s*([\w\d_]+)\s*\]/;
	my $name=$1;

	$comment=0 if $comment && s=^.*?\*/==;
	s=/\*.*?\*/|//.*$==g;
	next if $comment;
	$comment = s=/\*.*$==;

	next unless $name;

	$last = $_;

	my ($retType, $funcName, $params) = /\s*(.*?)\s*\b([\w\d_]+)\s*\((.*)\)/;
	next unless $retType && $funcName && defined $params;

	if (($retType eq "extern PyObject *") && ($params eq "PyObject *self, PyObject *args")) {
		$wrappersList.="\t{\"$name\", $funcName, METH_VARARGS, \"\"},\n";
		say $fh "extern PyObject *$funcName(PyObject *self, PyObject *args);";
		$last=0;
		next;
	}

	my %retType = getType $retType;
	next unless %retType;

	my @params=();
	my %array=();
	if ($params !~ /^\s*(?:void\s*)?$/) {
		@params=split /\,\s*/, $params;
		@params=map [/(.*)\s*\b([\w\d_]+)/], @params;
		for (@params) {
			next main unless @$_==2;
		}

		@params=map +{getType $_->[0], name=>"$_->[1]"}, @params;
		for (@params) {
			%array=(name=>$_->{name}, localVarsList=>[]) if $_->{isArray};
		}

		for (my $pos=$#params; $pos>=0; $pos--) {
			$params[$pos]{pos}=$pos;
			if (%array) {
				if ($params[$pos]{name} eq $array{name}) {
					next main unless ($pos==$#params);
					%array=(%array,%{pop @params});
					unshift $array{localVarsList}, "$array{name}Wr";
					next;
				}
				if ($params[$pos]{name} eq $array{name}."Cnt") {
					next main unless ($pos==$#params) && ($params[$pos]{cType} eq "int");
					pop @params;
					unshift $array{localVarsList}, "$array{name}CntWr";
					next;
				}
			}
			next main unless defined $params[$pos]{pyType};
		}
	}

	$last=0;

	#say STDERR Dumper @params;
	#say STDERR Dumper \%array;

	if ($print_headers) {
		say $_, " as $name";
	} else {
		if ($lastFile ne $ARGV) {
			say $fc "#include \"$ARGV\"";
			$lastFile=$ARGV;
		}
		my $cond                  = "argCnt" . (%array?">=":"==") . scalar @params;
		my $localVarsList         = join ", ", map("$_->{name}Wr", @params), @{$array{localVarsList}};
		my $retVarEq              = ($retType{pyFormat} ? "$retType{cType} retWr=" : "");
		my $commaRetVar           = ($retType{pyFormat} ? ", retWr" : "");
		my $paramsCnt             = scalar @params;
		my $objDecl               = (%array || @params ? "\n\t\tPyObject *obj;" : "");

		push @{$functions{$name}}, <<PARAMS . join("",map <<ARRAY, @params) . (%array?<<PARAMS_END:"") . <<EOF;
	if ($cond) {$objDecl
PARAMS
		obj=PyTuple_GET_ITEM(args, $_->{pos});
		$_->{cType} $_->{name}Wr = $_->{converter}(obj);
ARRAY
		int $array{name}CntWr=argCnt-$paramsCnt;
		$array{cType} $array{name}Wr[argCnt-$paramsCnt];
		for (int i=$paramsCnt; i<argCnt; i++) {
			obj=PyTuple_GET_ITEM(args, i);
			$array{name}Wr[i-$paramsCnt]=$array{converter}(obj);
		}
PARAMS_END
		if (!PyErr_Occurred()) {
			$retVarEq$funcName($localVarsList);
			if (PyErr_Occurred())
				return NULL;
			return Py_BuildValue("$retType{pyFormat}"$commaRetVar);
		}
		PyErr_Clear();
	}
EOF
	}

}

say $fc "";

for (keys %functions) {
	say $fh "extern PyObject *${_}Wrapper(PyObject *self, PyObject *args);";
	say $fc <<EOF, @{$functions{$_}}, <<EOF;
extern PyObject *${_}Wrapper(PyObject *self, PyObject *args) {
	int argCnt=PyTuple_Size(args);
EOF
	PyErr_SetString(PyExc_RuntimeError, "Wrong parameters");
	return NULL;
}
EOF
	$wrappersList.="\t{\"$_\", ${_}Wrapper, METH_VARARGS, \"\"},\n";
}


say STDERR "Skipping: $last" if $last;

say $fh <<EOF;

static PyMethodDef scriptWrappersList[] = {
$wrappersList	{NULL, NULL, 0, NULL}
};
EOF

unless ($print_headers) {
	close $fc;
	close $fh;
}
