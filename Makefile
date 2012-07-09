all: html/index.xhtml html/cookbook.xhtml

html/index.xhtml: xml/dact.xml
	xsltproc --xinclude xsl/html.xsl $< > $@

html/cookbook.xhtml: xml/cookbook.xml
	xsltproc --xinclude xsl/html.xsl $< > $@

clean:
	rm -f html/index.xhtml html/cookbook.xhtml

open: html/index.xhtml
	open html/index.xhtml