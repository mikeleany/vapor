package PGNParse;

use strict;

our $VERSION = '0.1';

sub new {
  my $class = shift;
  my $filename = shift;
  my $self = {};
  bless $self, $class;
  return $self->_open($filename);
}

sub _open {
  my $self = shift;
  my $filename = shift;
  my $handle;

  die "PGNParse object cannot be opened twice" if exists $self->{filename};

  $self->{filename} = $filename;
  open $handle, '<', $filename or die "$filename: $!";
  $self->{handle} = $handle;
  return $self;
}

sub close {
  my $self = shift;

  close $self->{handle} if defined $self->{handle};
  $self->{handle} = undef;
  return 1;
}

sub readnext {
  my $self = shift;
  my $f = $self->{handle};
  my $line;
  my %tags;
  my $movetext;
  my $result;

  $line = <$f> unless defined $line;
  return undef unless defined $line;

  while (defined $line && $line !~ m/\[.*\]/) {
    $line = <$f>;
  }
  return undef unless defined $line;

  while (defined $line && $line =~ m/\[(\S+)\s+"(.*)"\]/) {
    my $tag = $1;
    my $val = $2;
    $tags{$tag} = $val;
    $line = <$f>;
  }
  die "No tags" unless defined $tags{Result};
  $result = $tags{Result};

  while (defined $line) {
    $movetext .= $line;
    last if index($line, $result) >= 0;
    $line = <$f>;
  }

  $movetext =~ s/\d+\.//g;
  my @moves = split ' ', $movetext;
  pop @moves; # pop off result
  $movetext = join(',', @moves);
  $tags{MoveText} = $movetext;

  return \%tags;
}

1;
