#!/usr/bin/awk -f
func printst() {
	switch (ldepth) {
	case 1:
		printf("{ " \
		       ".vendor = 0x%s, " \
		       ".vendor_name = \"%s\", " \
		       ".dev_name = %s, " \
		       ".subsys_name = %s, " \
		       " }",
		       vendor, vendor_name, "NULL", "NULL")
		break
	case 2:
		printf("{ " \
		       ".vendor = 0x%s, " \
		       ".vendor_name = \"%s\", " \
		       ".dev = 0x%s, " \
		       ".dev_name = \"%s\", " \
		       ".subsys_name = %s, " \
		       " }",
		       vendor, vendor_name, dev,
		       dev_name, "NULL")
		break
	case 3:
		printf("{ " \
		       ".vendor = 0x%s, " \
		       ".vendor_name = \"%s\", " \
		       ".dev = 0x%s, " \
		       ".dev_name = \"%s\", " \
		       ".subvendor = 0x%s, " \
		       ".subdev = 0x%s, " \
		       ".subsys_name = \"%s\", " \
		       " }",
		       vendor, vendor_name, dev,
		       dev_name, subvendor, subdevice, subsystem_name)
		break
	}

	printf(",\n")
}

{
	ldepth = 0
	do {
		
		gsub("\\\\", "\\\\")
		gsub("\"", "\\\"")
		gsub("\\?", "\\\\?")
		# If comment or blank line
		if (/^[ \t]*(#.*)?$/)
			continue
		# If line is like:
		# vendor vendor_name
		else if (/^[^\t]/ && $0 !~ /^C/) {
			if (ldepth >= 1)
				printst()

			vendor = gensub(/^([0-9a-f]+)[ \t]+(.+)/, "\\1", 1)
			vendor_name = gensub(/^([0-9a-f]+)[ \t]+(.+)/, "\\2", 1)
			dev = dev_name = ""
			subvendor = subdevice = subsystem_name = ""

			ldepth = 1
		}
		# If line is like:
		# 	device device_name
		else if (/^\t[^\t]/) {
			if (ldepth >= 2)
				printst()

			dev = gensub(/^\t([0-9a-f]+)[ \t]+(.+)/, "\\1", 1)
			dev_name = gensub(/^\t([0-9a-f]+)[ \t]+(.+)/, "\\2", 1)
			subvendor = subdevice = subsystem_name = ""

			ldepth = 2
		}
		# If line is like:
		# 		subvendor subdevice subsystem_name
		else if (/^\t\t[^\t]/) {
			if (ldepth >= 3)
				printst()

			subvendor = gensub(/^\t\t([0-9a-f]+)[ \t]+([0-9a-f]+)[ \t]+(.+)/, "\\1", 1)
			subdevice = gensub(/^\t\t([0-9a-f]+)[ \t]+([0-9a-f]+)[ \t]+(.+)/, "\\2", 1)
			subsystem_name = gensub(/^\t\t([0-9a-f]+)[ \t]+([0-9a-f]+)[ \t]+(.+)/, "\\3", 1)

			ldepth = 3
		}
		else
			exit
	} while (getline > 0)
}
