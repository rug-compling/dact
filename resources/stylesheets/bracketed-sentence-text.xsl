<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  extension-element-prefixes="str exsl">

  <xsl:param name="showFilenames" select="0"/>

  <xsl:strip-space elements="entriesinfo"/>
  <xsl:strip-space elements="entries"/>
  <xsl:strip-space elements="entry"/>

  <xsl:output method="text" encoding="UTF-8"/>

  <xsl:template match="entriesinfo">
    <xsl:apply-templates />
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="corpus">
    <xsl:text>Corpus: </xsl:text>
    <xsl:apply-templates />
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="filter">
    <xsl:text>Filter: </xsl:text>
    <xsl:apply-templates />
      <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="date">
    <xsl:text>Date: </xsl:text>
    <xsl:apply-templates />
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="filename">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates />
      <xsl:text>&#x9;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="count">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates />
      <xsl:text>&#x9;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="sentence">
    <xsl:apply-templates />
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="bracket">
    <xsl:text>[</xsl:text>
    <xsl:apply-templates />
    <xsl:text>]</xsl:text>
  </xsl:template>

</xsl:stylesheet>
