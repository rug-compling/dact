all: html/index.xhtml

html/index.xhtml: xml/dact.xml
	xsltproc --xinclude xsl/html.xsl $< > $@

clean:
	rm -f html/index.xhtml

open: html/index.xhtml
	open html/index.xhtml