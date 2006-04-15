## Process this file with automake to produce Makefile.in

INSTALL_DOCU=CardCol.html.en CardCol.html.de CardCol.html.es \
             Røvhult.html.de Røvhult.html.en Røvhult.html.es \
	     Twopart.html.en Twopart.html.de Twopart.html.es \
	     Hearts.html.en Hearts.html.es Hearts.html.de Buraco.html.de \
	     Buraco.html.en Buraco.html.es Machiavelli.html.de \
	     Machiavelli.html.en Machiavelli.html.es Sgt.Mayor.html.en \
	     Sgt.Mayor.html.de Sgt.Mayor.html.es
EXTRA_DIST=$(INSTALL_DOCU)

docudir=$(datadir)/doc/@PACKAGE@

install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(docudir)
	@list='$(INSTALL_DOCU)'; for f in $$list ; do \
	  echo " $(INSTALL_DATA) $$f $(DESTDIR)$(docudir)/$$f"; \
	  $(INSTALL_DATA) $$f $(DESTDIR)$(docudir)/$$f; \
	 done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	@list='$(INSTALL_DOCU)'; for f in $$list ; do \
	  echo " rm -f $(DESTDIR)$(docudir)/$$f"; \
	  rm -f $(DESTDIR)$(docudir)/$$f; \
	 done
	rmdir $(DESTDIR)$(docudir)

