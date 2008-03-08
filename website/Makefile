# Makefile to maintain the website for cereja.
# $Id$

# NOTE:
#   - This Makefile is `called' by `../Makefile'.
#   - The following files are updated before `../Makefile' being called:
#     - ../VERSION
#     - ../cereja-$(<../VERSION)*.tar.bz2
#     - ../doc/manual.html

.PHONY: all upload upload-check
TARGETS = index.html manual.html

SED_SCRIPT_AD = -e '/@@AD@@/{' -e 'rAD' -e 'd' -e '}'
SED_SCRIPT_VERSION = -e "s/@@VERSION@@/$$(<../VERSION)/g"




all: $(TARGETS)

index.html: index.html.in ../VERSION AD Makefile
	sed $(SED_SCRIPT_VERSION) $(SED_SCRIPT_AD) $< >$@

manual.html: ../doc/manual.html AD Makefile
	sed $(SED_SCRIPT_AD) $< >$@




upload: all ,Ftpfile
	fteep --file=,Ftpfile --cache=+Ftpfile.cache
upload-check: all ,Ftpfile
	fteep --file=,Ftpfile --cache=+Ftpfile.cache --just-print

,Ftpfile: Ftpfile ../VERSION Makefile
	sed $(SED_SCRIPT_VERSION) $< >$@

# __END__
