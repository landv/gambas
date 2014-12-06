#!/usr/bin/gawk -f

BEGIN {
	class = "ERROR";
}

/GB_DECLARE/ {
	class = gensub(/GB_DECLARE\("([^"]+).*/, "\\1", "");
	gsub(/[\t ]+/, "", class);
}

match($0, "GB_.*" funcname) {
	printf class " ";
	# Only print once as there may be synonyms.
	exit;
}
