<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  xmlns:set="http://exslt.org/sets"
  extension-element-prefixes="str exsl">

  <xsl:param name="showFilenames" select="0"/>
  <xsl:param name="outputType" select="sentence" />

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

  <xsl:template match="entry">
    <xsl:choose>
      <xsl:when test="$outputType = 'kwic'">
        <xsl:apply-templates mode="kwic" />
      </xsl:when>
      <xsl:when test="$outputType = 'match'">
        <xsl:apply-templates mode="match" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates mode="sentence" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Sentence -->

  <xsl:template match="filename" mode="sentence">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates mode="sentence"/>
      <xsl:text>&#x9;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="count" mode="sentence">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates mode="sentence"/>
      <xsl:text>&#x9;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="sentence" mode="sentence">
    <xsl:apply-templates mode="sentence"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="bracket" mode="sentence">
    <xsl:text>[</xsl:text>
    <xsl:apply-templates mode="sentence"/>
    <xsl:text>]</xsl:text>
  </xsl:template>

  <!-- Match -->
    <xsl:template match="filename" mode="match">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates mode="match"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="count" mode="match"/>

  <xsl:template match="sentence" mode="match">
    <xsl:apply-templates select="bracket" mode="match"/>
  </xsl:template>

  <xsl:template match="bracket" mode="match">
    <xsl:if test="$showFilenames">
      <xsl:text>&#x9;</xsl:text>
    </xsl:if>
    <xsl:value-of select="."/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:apply-templates select="bracket" mode="match"/>
  </xsl:template>

  <!-- KWIC -->

  <xsl:template match="filename" mode="kwic">
    <xsl:if test="$showFilenames">
      <xsl:apply-templates mode="kwic"/>
      <xsl:text>&#xa;</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="count" mode="kwic" />

  <xsl:template match="sentence" mode="kwic">
    <xsl:apply-templates select="bracket" mode="kwic"/>
  </xsl:template>

  <xsl:template match="bracket" mode="kwic">
    <xsl:variable name="sentTextNodes" select="exsl:node-set(ancestor::sentence//text())"/>
    <xsl:text>&#x9;</xsl:text>
    <xsl:value-of select="str:concat(set:leading($sentTextNodes,./text()))" />
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>&#x9;&#x9;</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>&#x9;&#x9;&#x9;</xsl:text>
    <xsl:value-of select="str:concat(set:trailing($sentTextNodes,descendant-or-self::*[last()]/text()))" />
    <xsl:text>&#xa;</xsl:text>
    <xsl:apply-templates select="bracket" mode="kwic"/>
  </xsl:template>

</xsl:stylesheet>
