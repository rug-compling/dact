<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xhtml="http://www.w3.org/1999/xhtml">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <!-- adapted from Jirka Kosek's page at 
       http://www.xml.com/pub/a/2004/09/08/tree.html -->

  <xsl:template match="/">
    <tree>
      <xsl:apply-templates select="/conllx_ds/word"/>
    </tree>
  </xsl:template>

  <xsl:template match="word">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
        <xsl:value-of select="@rel"/>
        <xhtml:br/>
        <xsl:value-of select="@pos"/>
        <xhtml:br/>
        <xsl:value-of select="@form"/>
        </xhtml:p>
      </label>
      <xsl:if test="@morph">
        <tooltip>
          <xhtml:i>
            <xsl:value-of select="@morph"/>
          </xhtml:i>
        </tooltip>
      </xsl:if>
      <xsl:apply-templates select="word"/>
    </node>
  </xsl:template>
</xsl:stylesheet>


