## Process this file with automake to produce Makefile.in

SUBDIRS=.

DESTINATION=${DESTDIR}${datadir}/doc/$(PACKAGE)

EXTRA_DIST=CardCol.shtml.en CardCol.shtml.de CardCol.shtml.es \
	   CardCol.html.en CardCol.html.de CardCol.html.es \
	   Røvhult.html.en Røvhult.html.de Røvhult.html.es \
	   Twopart.html.en Twopart.html.de Twopart.html.es \
	   Hearts.html.en Hearts.html.es Hearts.html.de 

doc-install:
	test -d ${DESTINATION} || mkdir ${DESTINATION}
	cp ${EXTRA_DIST} ${DESTINATION}

doc-uninstall:
	files='$(EXTRA_DIST)'; for f in $$files; do rm ${DESTINATION}/$$f; done
	rmdir ${DESTINATION}
