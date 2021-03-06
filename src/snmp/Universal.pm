use strict;
no warnings;

# ------------------------------------------
package UNIVERSAL;
no strict;
no warnings;
# ------------------------------------------
#package UNIVERSAL;
sub _forw {
	my $self = shift;
	my $type = shift;
	my $only = shift;
	my $each = shift;
	if ($only and exists *{"$type\::"}->{$only}) {
		my $sub = "$type\::$only";
		$self->$sub(@_);
	} elsif ($each) {
		if (exists *{"$type\::"}->{ISA}) {
			foreach my $clas (@{*{"$type\::"}->{ISA}}) {
				$self->_forw($clas,$only,$each,@_);
			}
		}
		if (exists *{"$type\::"}->{$each}) {
			my $sub = "$type\::$each";
			$self->$sub(@_);
		}
	}
}
#package UNIVERSAL;
sub _fwcd {
	my $self = shift;
	my $type = shift;
	my $only = shift;
	my $each = shift;
	my $want = shift;
	my $have = $want;
	if ($only and exists *{"$type\::"}->{$only}) {
		my $sub = "$type\::$only";
		$have = $self->$sub(@_);
	} elsif ($each) {
		if (exists *{"$type\::"}->{ISA}) {
			foreach my $clas (@{*{"$type\::"}->{ISA}}) {
				$have = $self->_fwcd($clas,$only,$each,$want,@_);
				last unless $have eq $want;
			}
		}
		if ($have eq $want) {
			if (exists *{"$type\::"}->{$each}) {
				my $sub = "$type\::$each";
				$have = $self->$sub(@_);
			}
		}
	}
	return $have;
}
#package UNIVERSAL;
sub _init {
	my $self = shift;
	my $type = shift;
	$self->_forw($type,'only','init',@_);
}
#package UNIVERSAL;
sub _post {
	my $self = shift;
	my $type = shift;
	$self->_revs($type,undef,'post',@_);
}
#package UNIVERSAL;
sub _new {
	my $clas = shift;
	my $type = shift;
	my @isas = ();
	@isas = @{*{"$clas\::"}->{ISA}} if exists *{"$clas\::"}->{ISA};
	my $self;
	if (my $isa = shift @isas) {
		$self = $isa->_make($type,@_);
	} else {
		$self = {};
		bless $self,$type;
	}
	return $self;
}
#package UNIVERSAL;
sub _make {
	my $clas = shift;
	my $type = shift;
	my $self;
	if (exists *{"$clas\::"}->{make}) {
		$self = &{*{"$clas\::"}->{make}}($type,@_);
	} else {
		$self = $clas->_new($type,@_);
	}
	return $self;
}
#package UNIVERSAL;
sub new {
	my $type = shift;
	my $self = $type->_make($type,@_);
	$self->_init($type,@_);
	$self->_post($type,@_);
	return $self;
}
#package UNIVERSAL;
sub _revs {
	my $self = shift;
	my $type = shift;
	my $only = shift;
	my $each = shift;
	if ($only and exists *{"$type\::"}->{$only}) {
		my $sub = "$type\::$only";
		$self->$sub(@_);
	} elsif ($each) {
		if (exists *{"$type\::"}->{$each}) {
			my $sub = "$type\::$each";
			$self->$sub(@_);
		}
		if (exists *{"$type\::"}->{ISA}) {
			foreach my $clas (reverse @{*{"$type\::"}->{ISA}}) {
				$self->_revs($clas,$only,$each,@_);
			}
		}
	}
}
#package UNIVERSAL;
sub _rvcd {
	my $self = shift;
	my $type = shift;
	my $only = shift;
	my $each = shift;
	my $want = shift;
	my $have = $want;
	if ($only and exists *{"$type\::"}->{$only}) {
		my $sub = "$type\::$only";
		$have = $self->$sub(@_);
	} elsif ($each) {
		if (exists *{"$type\::"}->{$each}) {
			my $sub = "$type\::$each";
			$have = $self->$sub(@_);
		}
		if ($have eq $want) {
			if (exists *{"$type\::"}->{ISA}) {
				foreach my $clas (reverse @{*{"$type\::"}->{ISA}}) {
					$have = $self->_rvcd($clas,$only,$each,$want,@_);
					last unless $have eq $want;
				}
			}
		}
	}
	return $have;
}
#package UNIVERSAL;
sub _prep {
	my $self = shift;
	my $type = shift;
	$self->_revs($type,undef,'prep',@_);
}
#package UNIVERSAL;
sub _fini {
	my $self = shift;
	my $type = shift;
	$self->_revs($type,'cull','fini',@_);
}
#package UNIVERSAL;
sub put {
	my $self = shift;
	my $type = ref $self;
	$self->_prep($type,@_);
	$self->_fini($type,@_);
}
#package UNIVERSAL;
sub get {
	my $type = shift;
	my $self;
	if ($type->can('find')) {
		if ($self = $type->find(@_)) {
			if ($self->can('link')) {
				$self->link(@_);
			}
			return $self;
		}
	}
	$self = $type->new(@_);
	return $self;
}
#package UNIVERSAL;
sub _dump {
	my $self = shift;
	my $type = shift;
	$self->_forw($type,'info','dump',@_);
}
#package UNIVERSAL;
sub show {
	my $self = shift;
	my $type = ref $self;
	warn "Bad object of type $type, $self\n" if $self->{bad};
	print STDERR "\nShowing item $self\n-------------------------\n";
	$self->_dump($type,@_);
}
#package UNIVERSAL;
sub _tree {
	my $self = shift;
	my $type = shift;
	$self->_forw($type,undef,'tree',@_);
}
#package UNIVERSAL;
sub walk {
	my $self = shift;
	my ($crumb,$indent,$suffix) = @_;
	my $type = ref $self;
	$suffix .= ' ...' if exists $self->{crumb} and $self->{crumb} == $crumb;
	print STDERR "$indent$self$suffix\n";
	$self->_tree($type,@_);
	$self->{crumb} = $crumb;
}

# ------------------------------------------
package Base;
no warnings;
# ------------------------------------------
##package Base;
#our %use = ();
#our %put = ();
#our %dst = ();
#our $init_count = 0;
#our $fini_count = 0;
#our $dest_count = 0;
##package Base;
#sub showstats {
#	print STDERR "Base object stats:\n------------------\n";
#	printf STDERR "%6d object init calls\n", $Base::init_count;
#	printf STDERR "%6d object fini calls\n", $Base::fini_count;
#	printf STDERR "%6d object dest calls\n", $Base::dest_count;
#	print STDERR "------------------\n";
#	printf STDERR "%6d objects use\n",scalar(values %Base::use);
#	printf STDERR "%6d objects put\n",scalar(values %Base::put);
#	printf STDERR "%6d objects dst\n",scalar(values %Base::dst);
#	print STDERR "------------------\n";
#}
##package Base;
#sub init {
#	my $self = shift;
#	$use{$self} = 1;
#	$init_count++;
#	Base::showstats() unless ($init_count % 10);
#}
##package Base;
#sub fini {
#	my $self = shift;
#	delete $Base::use{$self};
#	$put{$self} = 1;
#	$fini_count++;
#	Base::showstats() unless ($fini_count % 10);
#}
##package Base;
#sub DESTROY {
#	my $self = shift;
#	delete $Base::put{$self};
#	$dst{$self} = 1;
#	$dest_count++;
#	Base::showstats() unless ($dest_count % 10);
#}

1;

__END__
