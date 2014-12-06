#!/usr/bin/gawk -f

# Extract comments from C/C++ source files (stdin)

BEGIN {
	do_record = 0;
	not_this = 0;
	symname = "";
	FS="";
}

/^[\t ]*\*?\*\/$/ {
	if (length(symname)) {
		print "G "symname "\n";
		do_record = 0;
	}
	not_this = 1;
}

/^BEGIN\_.*\(.*/ {
	if (do_record && length(symname) == 0) {
		s = gensub(/^BEGIN\_.*\(([^,)]+).*/, "\\1", "");
		if (length(s) == 0)
			print "ERROR\n";
		else
			print s "\n";
		do_record = 0;
	}
}

{
	if (do_record && !not_this)
		print gensub(/^[\t ]*/, " ", "");
	not_this = 0;
}

# The 'G' flag is important to distinguish intended Gambas documentation
# for the wiki (*.help files) and already existing C function commentary.
#
# The start marker may either be:
#
# 	^/**G$			# Note that this describes an *entire* line
#
# when it preceeds a symbol definition (BEGIN_*) or alternatively
#
# 	/**G Class Symbol$	# Here, only the end of line matters
#
# if it documents the Gambas symbol "Symbol" in "Class". This is intended
# for documentation of symbols that are not to be implemented in something
# that begins with BEGIN_*, like GB_CONSTANTs or GB_PROPERTY_SELFs. It can
# also be used for any other symbol type but both documentation types should
# not be used together on a single symbol.

/^\/\*\*G$/ {
	if (do_record)
		print "ERROR\n";
	symname = "";
	do_record = 1;
}

/[\t ]*\/\*\*G .+$/ {
	if (do_record)
		print "ERROR\n";
	symname = gensub(/\/\*\*G (.*)$/, "\\1", "");
	gsub(/^[\t ]*/, "", symname);
	do_record = 1;
}

END {
	if (do_record)
		print "ERROR\n";
}
