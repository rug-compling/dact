<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    
    <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/xhtml/docbook.xsl" />
    
    <xsl:output method="xml"
        encoding="UTF-8"
        indent="yes"/>
        
    <!-- Default CSS stylesheet -->
    <xsl:param name="html.stylesheet">screen.css</xsl:param>
    
    <xsl:param name="html.ext">.xhtml</xsl:param>
    
    <xsl:param name="toc.section.depth" select="1" />
    
    <xsl:param name="section.autolabel" select="1" />
    <xsl:param name="section.label.includes.component.label" select="1" />

    <!-- No 'title' atttributes. -->
    <xsl:template match="*" mode="html.title.attribute"/>   
</xsl:stylesheet>
