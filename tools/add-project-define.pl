#!/usr/bin/perl

use strict;
use warnings;
use File::Find;
use File::Copy;

# my @dirs = qw(../modules/base);
# my $definition = 'IVW_MODULE_BASE_API';
# my $definepat = 'modules/base/basemoduledefine.h';

# my @dirs = qw(../modules/glut);
# my $definition = 'IVW_MODULE_GLUT_API';
# my $definepat = 'modules/glut/glutmoduledefine.h';

# my @dirs = qw(../modules/opengl);
# my $definition = 'IVW_MODULE_OPENGL_API';
# my $definepat = 'modules/opengl/openglmoduledefine.h';

# my @dirs = qw(../modules/opencl);
# my $definition = 'IVW_MODULE_OPENCL_API';
# my $definepat = 'modules/opencl/openclmoduledefine.h';

# my @dirs = qw(../include/inviwo/core);
# my $definition = 'IVW_CORE_API';
# my $definepat = 'inviwo/core/inviwocoredefine.h';

# my @dirs = qw(../include/inviwo/qt/editor);
# my $definition = 'IVW_QTEDITOR_API';
# my $definepat = 'inviwo/qt/editor/inviwoqteditordefine.h';

# my @dirs = qw(../include/inviwo/qt/widgets);
# my $definition = 'IVW_QTWIDGETS_API';
# my $definepat = 'inviwo/qt/widgets/inviwoqtwidgetsdefine.h';

my $interesting = 'h';
my $skip_dirs = '\.svn|\.moc|\.obj|\.ui';

# Put project defintion in proper location (and #include to *define.h)

find(\&wanted, @dirs);

sub wanted {
    if ($_ =~ /^($skip_dirs)$/) {
        $File::Find::prune = 1;
        return;
    };
    if ((-f $_) and ($_ =~ /\.($interesting)$/)) {
        add_class_defintion($_);
    }
}

sub add_class_defintion {
    my $file = shift;
    print "Processing file:" . $File::Find::name . "\n";
    open F, $file or die "Can't open $file: $!";
    my @f = <F>;
    close F;

    my $tmpfile = $file . ".tmp";
    open F, ">", $tmpfile or die "Can't open $tmpfile for writing: $!";
    my $l = '';
    my $line = 1;
    my $do_replace = 0;
    my $outside = 1;
    my $add_include = 1;
    my $space_count = -1;
    foreach (@f) {
        $l = $_;
        # Search lines to replace with define
        if ($outside && !($l =~ m/::+/) && ($l =~ m/class \w+\s*(:|{)/)) {
            $l =~ s/class/class $definition/;
            print $l;
            $do_replace = 1;
        } elsif ($outside && !($l =~ m/::+/) && ($l =~ m/struct \w+\s*(:|{)/)) {
            $l =~ s/struct/struct $definition/;
            print $l;
            $do_replace = 1;
        } elsif ($outside && !($l =~ m/::+/) && ($l =~ m/template class \w+</) && ($l =~ m/>;$/)) {
            $l =~ s/template class/template class $definition/;
            print $l;
            $do_replace = 1;
        } elsif ($outside && !($l =~ m/::+/) && ($l =~ m/inline \w+/) && (($l =~ m/\);$/) || ($l =~ m/{/))) {
            $l =~ s/inline/inline $definition/;
            print $l;
            $do_replace = 1;
        } elsif ($outside && !($l =~ m/::+/) && ($l =~ m/\w+/) && !($l =~ m/namespace/) && (($l =~ m/\);$/) || ($l =~ m/{/))) {
            $l =~ /^(\s*)/;
            my $start_space_count = length($1);
            my $new_line = $l;
            $new_line =~ s/^\s+//;
            $l = ' ' x $start_space_count . $definition . ' ' . $new_line;
            print $l;
            $do_replace = 1;
        } elsif ($add_include && ($l =~ m/^#include/)) {
            $l = '#include "' . $definepat. '"' . "\n" . $l;
            print $l;
            $add_include = 0;
        } elsif ($add_include && ($l =~ m/^\w/)) {
            $l = '#include "' . $definepat. '"' . "\n\n" . $l;
            print $l;
            $add_include = 0;
        }
        
        #Search if we are inside a statement
        if (!$outside && ($l =~ m/}/)) {
            $l =~ /^(\s*)/;
            if($space_count == length($1)) {
                $outside = 1;
                $space_count = -1;
            }
        } elsif ($outside && ($l =~ m/{/) && !($l =~ m/namespace/)) {
            $l =~ /^(\s*)/;
            $space_count = length($1);
            $outside = 0;
        }  
        print F $l;
        $line++;
    }
    close F;
    if ($do_replace) {
        move($tmpfile, $file);
    } else {
        unlink($tmpfile);
    }
}
