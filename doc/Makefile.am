## Process this file with automake to produce Makefile.in

SUBDIRS=.

DESTINATION=${DESTDIR}${datadir}/doc/$(PACKAGE)

EXTRA_DIST=index.shtml index.html Rovhult.html Twopart.html

doc-install:
	test -d ${datadir}/doc/$(PACKAGE) || mkdir ${datadir}/doc/$(PACKAGE)
	cp ${EXTRA_DIST} ${DESTINATION}

doc-uninstall:
	files='$(EXTRA_DIST)'; for f in $$files; do rm ${DESTINATION}/$$f; done
	rmdir ${DESTINATION}