tests = \
    install.suite/install_root.test
EXTRA_DIST = $(tests)

TESTS = run_tests
check_PROGRAMS = run_tests t
run_tests_SOURCES = $(top_srcdir)/src/test/run_tests.c
t_SOURCES = $(top_srcdir)/src/test/t.c t_calls.c
t_LDADD = $(top_builddir)/lib/lib23package/lib23package.la

t_calls.c: $(top_srcdir)/src/test/make_t_calls.sh $(tests)
	$^ >$@
