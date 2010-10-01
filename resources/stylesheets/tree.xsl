<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:saxon="http://icl.com/saxon"
  xmlns:set="http://exslt.org/sets"
  xmlns:exslt="http://exslt.org/common"
  extension-element-prefixes="saxon set exslt">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:param name='expr'>/..</xsl:param> <!-- matcht nooit -->

  <!-- adapted from Jirka Kosek's page at 
       http://www.xml.com/pub/a/2004/09/08/tree.html -->

  <xsl:variable name="newexpr">
    <xsl:choose>
      <xsl:when test="exslt:object-type(saxon:evaluate($expr)) = 'node-set'">
        <xsl:value-of select="$expr"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>/..</xsl:text>
      </xsl:otherwise>
    </xsl:choose>    
  </xsl:variable>

  <xsl:variable name="selectedNodes" select="saxon:evaluate($newexpr)" />

  <xsl:template name="node-active">
      <xsl:choose>
        <xsl:when test="set:intersection($selectedNodes, .)">
          <xsl:text>yes</xsl:text>
        </xsl:when>
      </xsl:choose>
  </xsl:template>

  <xsl:template match="/">
    <xsl:apply-templates select="/alpino_ds/node" mode="xml2layout"/>
  </xsl:template>
  
  <!-- Add layout attributes to leaf nodes -->
  <xsl:template match="node" mode="xml2layout">
    <node>
      <xsl:attribute name="active">
        <xsl:call-template name="node-active" />
      </xsl:attribute>
      <xsl:copy-of select="@*"/>
	  <line>
		<xsl:value-of select="@index"/>
		<xsl:if test = "@index and (@cat|@pt)">
		  <xsl:text>:</xsl:text>
		</xsl:if>
		<xsl:value-of select="@cat|@pt"/>
	  </line>
	  <line>
	    <xsl:value-of select="@lemma"/>
	  </line>
      <xsl:apply-templates select="node" mode="xml2layout" />
    </node>
  </xsl:template>
</xsl:stylesheet>


