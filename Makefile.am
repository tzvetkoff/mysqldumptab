ACLOCAL_AMFLAGS = -Im4

SUBDIRS = src test
dist_doc_DATA = AUTHORS ChangeLog README.md LICENSE.md

maintainer-clean-local:
	-rm -f aclocal.m4 config.h.in configure config
	-find ${builddir} -name Makefile.in -exec rm -f '{}' ';'

.PHONY: bootstrap
bootstrap: maintainer-clean
	./bootstrap
