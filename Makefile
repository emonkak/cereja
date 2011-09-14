# Makefile for cereja.
# BUGS: assumes GNU make.
__ID__=$$Id$$
# Configurations  #{{{1

ON_COLINUX_P=$(filter colinux,$(ENV_WORKING))
CC=$(if $(ON_COLINUX_P),i586-mingw32msvc-gcc,gcc)
STRIP=$(if $(ON_COLINUX_P),i586-mingw32msvc-strip,strip)
CFLAGS=$(CFLAGS_REQUIRED) $(CFLAGS_OPTIONAL)
CFLAGS_REQUIRED=-pedantic -Wall -Werror -std=gnu99 -mno-cygwin -mwindows
CFLAGS_ADDITIONALS=-Wextra -Wcast-align -Wcast-qual \
  -Wfloat-equal -Wformat=2 -Wpointer-arith -Wsign-compare \
  -Wstrict-aliasing=2 -Wwrite-strings \
  -Wno-conversion
CFLAGS_OPTIONAL=-pipe -g -O0
CPPFLAGS=-I./src -I./lib -I$(LUA_DIR)/src

LUA_DIR=./lua

UTF8API_HEADERS=src/utf8api.h src/utf8api.h.gen
LUA_HEADERS=$(LUA_DIR)/src/lauxlib.h $(LUA_DIR)/src/lua.h \
            $(LUA_DIR)/src/luaconf.h $(LUA_DIR)/src/lualib.h
CEREJA_HEADERS=src/cereja.h src/version.h
ALL_HEADERS=$(UTF8API_HEADERS) $(LUA_HEADERS) $(CEREJA_HEADERS)

LUA_TARGETS=liblua.dll liblua.a
CEREJA_TARGETS=\
  libutf8api.dll libutf8api.a \
  libcereja.dll libcereja.a \
  cereja.exe $(EXTENSIONS)
ALL_TARGETS=$(LUA_TARGETS) $(CEREJA_TARGETS)
EXTENSIONS=\
  lib/app/snarl.dll \
  lib/shell/__init__.dll \
  lib/shell/tray.dll lib/shell/tray-hook.dll \
  lib/shell/window.dll \
  lib/ui/hotkey.dll \
  lib/ui/ime.dll \
  lib/ui/key.dll \
  lib/ui/monitor.dll \
  lib/ui/tray.dll \
  lib/ui/window.dll \
  lib/ui/sound.dll \
  lib/ui/system.dll

.PHONY: \
  all \
  build debug-build release-build \
  doc \
  test \
  \
  archive _version-check \
  dist dist-bin dist-src \
  snapshot snapshot-bin snapshot-src \
  \
  clean cereja-clean lua-clean distclean \
  strip \
  tags \
  rsync \
  \
  website \
  website-upload \
  website-upload-check

all: debug-build








# build  #{{{1

build: cereja.exe $(EXTENSIONS)
debug-build: build
release-build:
	$(MAKE) 'CFLAGS_OPTIONAL=$(CFLAGS_OPTIONAL) -O3' build

cereja_SOURCES=src/main.c
cereja.exe: $(cereja_SOURCES:.c=.o) libcereja.dll liblua.dll libutf8api.dll
	$(CC) $(CFLAGS) -o $@ \
	  $(filter-out lib%.dll,$^) \
	  -L. $(patsubst lib%.dll,-l%,$(filter lib%.dll,$^))

libcereja_SOURCES=src/api.c src/builtins.c
libcereja.dll: $(libcereja_SOURCES:.c=.o) liblua.dll libutf8api.dll
	$(CC) $(CFLAGS) -o $@ -shared \
	  -Wl,--out-implib=$(@:.dll=.a) \
	  -Wl,--whole-archive $(filter-out lib%.dll,$^) \
	  -Wl,--no-whole-archive \
	    -L. $(patsubst lib%.dll,-l%,$(filter lib%.dll,$^))

liblua_SOURCES_CORE=lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
                    llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c \
                    lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c
liblua_SOURCES_LIB=lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c \
                   ltablib.c lstrlib.c loadlib.c linit.c
liblua_SOURCES=$(addprefix $(LUA_DIR)/src/, \
                           $(liblua_SOURCES_CORE) $(liblua_SOURCES_LIB))
liblua.dll: $(liblua_SOURCES:.c=.o) libutf8api.dll
	$(CC) $(CFLAGS) -o $@ -shared \
	  -Wl,--out-implib=$(@:.dll=.a) \
	  -Wl,--whole-archive $(filter-out lib%.dll,$^) \
	  -Wl,--no-whole-archive \
	    -L. $(patsubst lib%.dll,-l%,$(filter lib%.dll,$^))
$(liblua_SOURCES:.c=.o): \
  CFLAGS_REQUIRED+=-D UNICODE -D _UNICODE -D 'LUA_USER_H="utf8api.h"'

libutf8api_SOURCES=src/utf8api.c
libutf8api.dll: $(libutf8api_SOURCES:.c=.o)
	$(CC) $(CFLAGS) -o $@ -shared \
	  -Wl,--out-implib=$(@:.dll=.a) \
	  -Wl,--whole-archive $^ \
	  -Wl,--no-whole-archive \
	  -lwinmm
src/utf8api.h.gen: src/utf8api.h
	sed -ne 's/.*Utf8_PUBLIC([^()]*) *\([A-Za-z0-9_]*\)U(.*/#undef \1\n#define \1 \1U\n#define \1T \1W/; t L; b; :L; p' <$< >$@

define extension_prerequisites
$(1): $(1:.dll=.o)
endef
$(foreach i,$(EXTENSIONS),$(eval $(call extension_prerequisites,$(i))))
lib/shell/__init__.dll: EXTRA_LIBS = powrprof
lib/shell/tray.dll: EXTRA_LIBS = ole32
lib/ui/ime.dll: EXTRA_LIBS = imm32
lib/ui/sound.dll: EXTRA_LIBS = winmm
lib/ui/system.dll: EXTRA_LIBS = iphlpapi ws2_32
$(EXTENSIONS): libcereja.dll liblua.dll libutf8api.dll

$(cereja_SOURCES:.c=.o) \
$(EXTENSIONS:.dll=.o) \
$(libcereja_SOURCES:.c=.o) \
$(libutf8api_SOURCES:.c=.o): CFLAGS += $(CFLAGS_ADDITIONALS)

$(cereja_SOURCES:.c=.o): $(CEREJA_HEADERS) $(LUA_HEADERS) $(UTF8API_HEADERS)
$(EXTENSIONS:.dll=.o): $(CEREJA_HEADERS) $(LUA_HEADERS) $(UTF8API_HEADERS)
lib/shell/tray.o lib/ui/tray.o: lib/shell/tray.h
$(libcereja_SOURCES:.c=.o): $(CEREJA_HEADERS) $(LUA_HEADERS) $(UTF8API_HEADERS)
$(liblua_SOURCES:.c=.o): $(LUA_HEADERS) $(UTF8API_HEADERS)
$(libutf8api_SOURCES:.c=.o): $(UTF8API_HEADERS)








# test  #{{{1

ALL_TESTS=$(ALL_TESTS_C) $(ALL_TESTS_LUA)
ALL_TESTS_C=test utf8api api builtins_c builtins_c2
ALL_TESTS_LUA=builtins_lua lib/ui/key
test: all $(ALL_TESTS:%=tests/%.ok)

%.ok: %.result %.expected
	diff -u $*.expected $*.result
	touch $@
%.result: %.exe
	./$< >$@
%.result: %.lua
	./cereja.exe --test $< >$@
%.exe: %.o
	$(CC) $(CFLAGS) -o $@ \
	  $(filter-out lib%.dll,$^) \
	  -L. $(patsubst lib%.dll,-l%,$(filter lib%.dll,$^))
%.c.gen: %.c
	./tests/test-gen.sh $< >$@

define test_prerequisites_c
tests/$(1).ok: tests/$(1).result tests/$(1).expected
tests/$(1).result: tests/$(1).exe
tests/$(1).exe: tests/$(1).o tests/test-framework.o libcereja.dll liblua.dll libutf8api.dll
tests/$(1).o: tests/$(1).c.gen
tests/$(1).c.gen: tests/$(1).c tests/test-gen.sh
endef
$(foreach i,$(ALL_TESTS_C),$(eval $(call test_prerequisites_c,$(i))))
define test_prerequisites_lua
tests/$(1).ok: tests/$(1).result tests/$(1).expected
tests/$(1).result: tests/$(1).lua cereja.exe libcereja.lua
endef
$(foreach i,$(ALL_TESTS_LUA),$(eval $(call test_prerequisites_lua,$(i))))
ALL_TEST_SOURCES=$(patsubst %, tests/%.c, $(ALL_TESTS_C)) \
                 tests/builtins_c_dll.c tests/builtins_lua_dll.c
$(ALL_TEST_SOURCES:.c=.o): $(ALL_HEADERS) tests/test-framework.h

tests/builtins_c.result: tests/builtins_c_dll.dll
tests/builtins_c_dll.dll: tests/builtins_c_dll.o libcereja.dll liblua.dll libutf8api.dll
tests/builtins_lua.result: tests/builtins_lua_dll.dll
tests/builtins_lua_dll.dll: tests/builtins_lua_dll.o \
                            libcereja.dll liblua.dll libutf8api.dll
tests/builtins_c2.result: tests/builtins_c_dll.dll tests/builtins_lua.ok








# doc  #{{{1

doc: doc/manual.html
doc/manual.html: doc/manual.xml doc/manual.xslt VERSION
	xsltproc \
	  --param MTIME "'$$(date -r $< +%Y-%m-%dT%H:%M:%S%:z)'" \
	  --param VERSION "'$$(<VERSION)'" \
	  -o $@ doc/manual.xslt doc/manual.xml








# website  #{{{1

WEBSITE_DONT_MAKE_DIST=f
website: VERSION doc
	$(MAKE) -C website
website-upload website-upload-check: VERSION doc
	if [ '$(WEBSITE_DONT_MAKE_DIST)' != 't' ]; then $(MAKE) dist; fi
	$(MAKE) -C website $(subst website-,,$@)








# archive  #{{{1
# common  #{{{2
# Abbreviations:
#   SVO: set via other target or command-line
#   EXPAT: EXcluding PATtern

ARCHIVE_NAME=# SVO; cereja-{VERSION} or cereja-{VERSION}+{DATETIME}
ARCHIVE_TREE_ISH=# SVO; tag-name or HEAD
ARCHIVE_TARGETS=# SVO; release-build test strip doc or nop
ARCHIVE_TARGET_TEST=$(if $(ON_COLINUX_P),,test)
ARCHIVE_EXPAT=# SVO
ARCHIVE_TEMP=,,archive-exclude
# EXPATs  #{{{
ARCHIVE_EXPAT_DEFAULT=\( \
                            -name '.??*' \
                        -or -name '+*' \
                        -or -name ',*' \
                        -or -name '*~' \
                        -or -name '*.c.gen' \
                        -or -name '*.h.gen' \
                        -or -name '*.o' \
                        -or -name '*.ok' \
                        -or -name '*.result' \
                        -or -name '*.tar.bz2' \
                      \)
ARCHIVE_EXPAT_BINARY=\( \
                       -type f \( \
                         -regex '[^/]*/[^/]*' \
                         -not -name 'cereja.exe' \
                         -not -name 'lib*.dll' \
                         -not -name 'lib*.lua' \
                       \) \
                       -or -type d \( \
                         -name 'src' \
                         -or -name 'lua' \
                         -or -name 'lunit' \
                         -or -name 'tests' \
                         -or -name 'website' \
                       \) \
                       -or -type f \( \
                         -regex '[^/]*/lib/.*\.[ch]' \
                         -or -name '*.xml' \
                         -or -name '*.xslt' \
                       \) \
                     \)
ARCHIVE_EXPAT_SOURCE=-false
#}}}

archive:
	@echo '# check configurations.'
	if [ -z '$(ARCHIVE_NAME)' ]; then \
          echo 'archive: ARCHIVE_NAME is required'; \
	  false; \
	fi
	if [ -z '$(ARCHIVE_TREE_ISH)' ]; then \
          echo 'archive: ARCHIVE_TREE_ISH is required'; \
	  false; \
	fi
	@echo '# remove garbages.'
	rm -rf $(ARCHIVE_TEMP) $(ARCHIVE_NAME) $(ARCHIVE_NAME).tar.bz2
	@echo '# extract source.'
	git archive --prefix=$(ARCHIVE_NAME)/ $(ARCHIVE_TREE_ISH) \
	  | tar xf -
	@echo '# does some work on the source.'
	if [ -n '$(ARCHIVE_TARGETS)' ]; then \
	  $(MAKE) -C $(ARCHIVE_NAME) $(ARCHIVE_TARGETS); \
	fi
	@echo '# list files and directories which should be excluded.'
	find $(ARCHIVE_NAME) $(ARCHIVE_EXPAT) >$(ARCHIVE_TEMP)
	cat $(ARCHIVE_TEMP)
	@echo '# make archive.'
	tar jcvfX $(ARCHIVE_NAME).tar.bz2 $(ARCHIVE_TEMP) $(ARCHIVE_NAME)
	@echo '# remove garbages.'
	rm -rf $(ARCHIVE_TEMP) $(ARCHIVE_NAME)




# snapshot  #{{{2
DATETIME:=$(if $(DATETIME),$(DATETIME),$(shell date '+%Y%m%dT%H%M%S'))
snapshot:
	$(MAKE) DATETIME='$(DATETIME)' snapshot-bin snapshot-src
snapshot-bin:
	$(MAKE) \
	  ARCHIVE_NAME='cereja-$(DATETIME)' \
	  ARCHIVE_TREE_ISH='HEAD' \
	  ARCHIVE_TARGETS='release-build $(ARCHIVE_TARGET_TEST) strip doc' \
	  ARCHIVE_EXPAT="$(ARCHIVE_EXPAT_DEFAULT) \
	             -or $(ARCHIVE_EXPAT_BINARY)" \
	  archive
snapshot-src:
	$(MAKE) \
	  ARCHIVE_NAME='cereja-$(DATETIME)-src' \
	  ARCHIVE_TREE_ISH='HEAD' \
	  ARCHIVE_TARGETS='' \
	  ARCHIVE_EXPAT="$(ARCHIVE_EXPAT_DEFAULT) \
	             -or $(ARCHIVE_EXPAT_SOURCE)" \
	  archive




# dist  #{{{2
VERSION=# SVO

dist: dist-bin dist-src

dist-bin: _version-check
	$(MAKE) \
	  ARCHIVE_NAME='cereja-$(VERSION)' \
	  ARCHIVE_TREE_ISH='v$(VERSION)' \
	  ARCHIVE_TARGETS='release-build $(ARCHIVE_TARGET_TEST) strip doc' \
	  ARCHIVE_EXPAT="$(ARCHIVE_EXPAT_DEFAULT) \
	             -or $(ARCHIVE_EXPAT_BINARY)" \
	  archive

dist-src: _version-check
	$(MAKE) \
	  ARCHIVE_NAME='cereja-$(VERSION)-src' \
	  ARCHIVE_TREE_ISH='v$(VERSION)' \
	  ARCHIVE_TARGETS='' \
	  ARCHIVE_EXPAT="$(ARCHIVE_EXPAT_DEFAULT) \
	             -or $(ARCHIVE_EXPAT_SOURCE)" \
	  archive

_version-check:
	if [ -z '$(VERSION)' ]; then \
	  echo 'VERSION is required.'; \
	  false; \
	fi








# Misc. targets  #{{{1

clean: cereja-clean
cereja-clean:
	rm -f $(CEREJA_TARGETS) \
	      `find \( -name '$(subst ./,,$(LUA_DIR))' -prune \) \
	        -or \(     -name ',*' \
	               -or -name '*.c.gen' \
	               -or -name '*.h.gen' \
	               -or -name '*.html' \
	               -or -name '*.o' \
	               -or -name '*.ok' \
	               -or -name '*.result' \
	            \) \
	            -print`
lua-clean:
	rm -f $(LUA_TARGETS) \
	      `find '$(LUA_DIR)' -name '*.o'`
distclean: cereja-clean lua-clean
	rm -f `find -name '*~'`


strip:
	$(STRIP) --strip-unneeded $(ALL_TARGETS)

tags:
	ctags `find -name '*.[ch]' -or -name '*.lua'`




VERSION: src/version.h
	sed -n \
	    -e '/ Crj_VERSION /{' \
	    -e '  s/^.* Crj_VERSION "\([^"]*\)".*$$/\1/' \
	    -e '  p' \
	    -e '  q' \
	    -e '}' \
	    <$< >$@




# To avoid unnecessary copying:
# - don't use -t (perserve times).
# - limit to only some types which are enough to test.
rsync:
	rsync -rlpgoD -u \
	  --include '*.exe' \
	  --include '*.dll' \
	  --include '*.lua' \
	  --include '*.html' \
	  --include '*/' \
	  --exclude '*' \
	  /home/kana/freq/latest/working/cereja/ \
	  /c/cygwin/home/kana/freq/latest/working/cereja








# Common implicit rules  #{{{1

%.o: %.c.gen
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ -x c $<
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.d: %.c.gen
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -x c $< \
	  | sed -e '1s,^[^:]*[ :]*,$*.o $@: ,g' >$@
%.d: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< \
	  | sed -e '1s,^[^:]*[ :]*,$*.o $@: ,g' >$@

%.dll: %.o
	$(CC) $(CFLAGS) -o $@ -shared \
	  -Wl,--whole-archive $(filter-out lib%.dll,$^) \
	  -Wl,--no-whole-archive \
	    -L. $(patsubst lib%.dll,-l%,$(filter lib%.dll,$^)) \
	        $(addprefix -l,$(EXTRA_LIBS))








# __END__  #{{{1
# vim: foldmethod=marker
