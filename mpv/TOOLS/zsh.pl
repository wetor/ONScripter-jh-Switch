#!/usr/bin/perl

# Generate ZSH completion

#
# This file is part of mpv.
#
# mpv is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# mpv is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
#

use strict;
use warnings;
use warnings FATAL => 'uninitialized';

my $mpv = $ARGV[0] || 'mpv';

my @opts = parse_main_opts('--list-options', '^ (\-\-[^\s\*]*)\*?\s*(.*)');

die "Couldn't find any options" unless (@opts);

my @ao = parse_opts('--ao=help', '^  ([^\s\:]*)\s*(.*)');
my @vo = parse_opts('--vo=help', '^  ([^\s\:]*)\s*(.*)');

my @af = parse_opts('--af=help', '^  ([^\s\:]*)\s*(.*)');
my @vf = parse_opts('--vf=help', '^  ([^\s\:]*)\s*(.*)');

my @protos = parse_opts('--list-protocols', '^ ([^\s]*)');

my ($opts_str, $ao_str, $vo_str, $af_str, $vf_str, $protos_str);

$opts_str .= qq{  '$_' \\\n} foreach (@opts);
chomp $opts_str;

$ao_str .= qq{      '$_' \\\n} foreach (@ao);
chomp $ao_str;

$vo_str .= qq{      '$_' \\\n} foreach (@vo);
chomp $vo_str;

$af_str .= qq{      '$_' \\\n} foreach (@af);
chomp $af_str;

$vf_str .= qq{      '$_' \\\n} foreach (@vf);
chomp $vf_str;

$protos_str = join(' ', @protos);

my $runtime_completions = <<'EOS';
  profile|show-profile)
    local -a profiles
    local current
    for current in "${(@f)$($words[1] --profile=help)}"; do
      current=${current//\*/\\\*}
      current=${current//\:/\\\:}
      current=${current//\[/\\\[}
      current=${current//\]/\\\]}
      if [[ $current =~ $'\t'([^$'\t']*)$'\t'(.*) ]]; then
        if [[ -n $match[2] ]]; then
          current="$match[1][$match[2]]"
        else
          current="$match[1]"
        fi
        profiles=($profiles $current)
      fi
    done
    if [[ $state == show-profile ]]; then
      # For --show-profile, only one allowed
      if (( ${#profiles} > 0 )); then
        _values 'profile' $profiles && rc=0
      fi
    else
      # For --profile, multiple allowed
      profiles=($profiles 'help[list profiles]')
      _values -s , 'profile(s)' $profiles && rc=0
    fi
  ;;

  audio-device)
    local -a audio_devices
    local current
    for current in "${(@f)$($words[1] --audio-device=help)}"; do
      current=${current//\*/\\\*}
      current=${current//\:/\\\:}
      current=${current//\[/\\\[}
      current=${current//\]/\\\]}
      if [[ $current =~ '  '\'([^\']*)\'' \('(.*)'\)' ]]; then
        audio_devices=($audio_devices "$match[1][$match[2]]")
      fi
    done
    audio_devices=($audio_devices 'help[list audio devices]')
    _values 'audio device' $audio_devices && rc=0
  ;;
EOS
chomp $runtime_completions;

my $tmpl = <<"EOS";
#compdef mpv

# For customization, see:
#  https://github.com/mpv-player/mpv/wiki/Zsh-completion-customization

local curcontext="\$curcontext" state state_descr line
typeset -A opt_args

local -a match mbegin mend
local MATCH MBEGIN MEND

# By default, don't complete URLs unless no files match
local -a tag_order
zstyle -a ":completion:*:*:\$service:*" tag-order tag_order || \
  zstyle  ":completion:*:*:\$service:*" tag-order '!urls'

local rc=1

_arguments -C -S \\
$opts_str
  '*:files:->mfiles' && rc=0

case \$state in
  ao)
    _values -s , 'audio outputs' \\
$ao_str
    && rc=0
  ;;

  vo)
    _values -s , 'video outputs' \\
$vo_str
    && rc=0
  ;;

  af)
    _values -s , 'audio filters' \\
$af_str
    && rc=0
  ;;

  vf)
    _values -s , 'video filters' \\
$vf_str
    && rc=0
  ;;

$runtime_completions

  files)
    compset -P '*,'
    compset -S ',*'
    _files -r ',/ \\t\\n\\-' && rc=0
  ;;

  mfiles)
    local expl
    _tags files urls
    while _tags; do
      _requested files expl 'media file' _files && rc=0
      if _requested urls; then
        while _next_label urls expl URL; do
          _urls "\$expl[@]" && rc=0
          compadd -S '' "\$expl[@]" $protos_str && rc=0
        done
      fi
      (( rc )) || return 0
    done
  ;;
esac

return rc
EOS

print $tmpl;

sub parse_main_opts {
    my ($cmd, $regex) = @_;

    my @list;
    my @lines = call_mpv($cmd);

    foreach my $line (@lines) {
        my ($name, $desc) = ($line =~ /^$regex/) or next;

        next if ($desc eq 'removed' || $desc eq 'alias');

        if ($desc =~ /^Flag/) {

            push @list, $name;

            $name =~ /^--(.*)/;
            if ($1 !~ /^(\{|\}|v|list-options|really-quiet|no-.*)$/) {
                push @list, "--no-$1";
            }

        } elsif ($desc =~ /^Print/) {

            push @list, $name;

        } else {

            # Option takes argument

            my $entry = $name;

            $desc =~ s/\:/\\:/g;
            $entry .= "=-:$desc:";

            if ($desc =~ /^Choices\\: ([^(]*)/) {
                my $choices = $1;
                $choices =~ s/ +$//; # strip trailing space
                $entry .= "($choices)";

                # If "no" is one of the choices, it can also be
                # negated like a flag (--no-whatever).
                if ($choices =~ /\bno\b/) {
                    $name =~ s/^--/--no-/;
                    push @list, $name;
                }
            } elsif ($line =~ /\[file\]/) {
                $entry .= '->files';
            } elsif ($name =~ /^--(ao|vo|af|vf|profile|show-profile|audio-device)$/) {
                $entry .= "->$1";
            }
            push @list, $entry;
        }
    }

    # Sort longest first, because zsh won't complete an option listed
    # after one that's a prefix of it.
    @list = sort {
        $a =~ /([^=]*)/; my $ma = $1;
        $b =~ /([^=]*)/; my $mb = $1;

        length($mb) <=> length($ma)
    } @list;

    return @list;
}

sub parse_opts {
    my ($cmd, $regex) = @_;

    my @list;
    my @lines = call_mpv($cmd);

    foreach my $line (@lines) {
        if ($line !~ /^$regex/) {
            next;
        }

        my $entry = $1;

        if (defined $2) {
            my $desc = $2;
            $desc =~ s/\:/\\:/g;
            $entry .= "[$desc]";
        }

        push @list, $entry
    }

    return @list;
}

sub call_mpv {
    my ($cmd) = @_;
    my $output = `"$mpv" --no-config $cmd`;
    if ($? == -1) {
        die "Could not run mpv: $!";
    } elsif ((my $exit_code = $? >> 8) != 0) {
        die "mpv returned $exit_code with output:\n$output";
    }
    return split /\n/, $output;
}
