#!/usr/bin/awk -f

function setdeps(arr, was, inc, idx, fs,	l, r, i, j, tmp, flag) {
	if (!fs)
		return idx

	if (was[arr[fs]])
		return idx

	was[fs] = 1
	l = idx

	while (getline <fs > 0) {
		if ($0 !~ /^[ \t]*#[ \t]*include[ \t]+\"/)
			continue

		split($0, tmp, "\"")
		s = tmp[2]

		if (inc)
			arr[idx] = inc "/"

		arr[idx] = arr[idx] s

		if (was[arr[idx]])
			continue

		idx++
	}

	r = idx

	for (i = l; i < r; i++) {
		flag = 0
		for (j in was) {
			if (j == arr[i]) {
				flag = 1
				break
			}
		}

		if (flag)
			continue

		idx = setdeps(arr, was, inc, idx, arr[i])
	}

	return idx
}

function getdeps(s, inc, arr,	was, idx) {
	idx = 1

	idx = setdeps(arr, was, inc, idx, s)

	return idx - 1
}

function join(arr, len, sep) {
	if (len != 0)
		s = arr[1]

	for (i = 2; i <= len; i++)
		s = s sep arr[i]

	return s
}

{
	if (!out)
		out = "/dev/stdout"
	else
		printf("") > out

	for (i in ARGV) {
		if (ARGV[i] ~ /\.h$/) {
			src = ARGV[i]

			len = getdeps(src, inc, arr)
			hdrs = join(arr, len, " ")

			printf("%s: %s\n", src, hdrs) >> out
		} else if (ARGV[i] ~ /\.c$/) {
			src = ARGV[i]

			obj = src
			gsub(/[^/]+\//, "", obj)
			sub(/\.c/, ".o", obj)

			len = getdeps(src, inc, arr)
			hdrs = join(arr, len, " ")

			if (build)
				printf("%s/", build) >> out

			printf("%s: %s %s\n", obj, src, hdrs) >> out
		}
	}

	exit
}

