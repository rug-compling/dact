all: html/index.xhtml

html/index.xhtml: xml/dact.xml
	xsltproc --nonet --xinclude xsl/html.xsl $< > $@
