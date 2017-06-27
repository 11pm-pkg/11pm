tests = \
    install.suite/install_root.test
EXTRA_DIST = $(tests)

TESTS = $(top_builddir)/src/test/run_tests
check_PROGRAMS = t
t_SOURCES = t.c t_calls.c
t_CFLAGS = -I$(top_srcdir)/lib/lib23package
t_LDADD = $(top_builddir)/lib/lib23package/lib23package.la

BUILT_SOURCES = t.c t_calls.c
CLEANFILES = $(BUILT_SOURCES)
t.c: $(top_srcdir)/src/test/t.c
	ln -s $< $@
t_calls.c: $(top_srcdir)/src/test/make_t_calls.sh $(tests:=.sh)
	$^ >$@
