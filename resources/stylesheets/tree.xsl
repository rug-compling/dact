<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xhtml="http://www.w3.org/1999/xhtml">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <!-- adapted from Jirka Kosek's page at 
       http://www.xml.com/pub/a/2004/09/08/tree.html -->

  <xsl:template match="/">
    <xsl:apply-templates select="/alpino_ds/node"/>
  </xsl:template>

  <!-- Interior nodes -->
  <xsl:template match="node[node]">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
        <xsl:value-of select="@rel"/>
        <!-- SoNaR annotation for semantic roles -->
        <xsl:if test="@pb">
          <xsl:text>/</xsl:text>
          <xhtml:font color="purple">
            <xsl:value-of select="@pb" />
          </xhtml:font>
        </xsl:if>
        <xhtml:br/>
        <xsl:value-of select="@index"/>
        <xsl:if test = "@index and (@cat|@pt)">
          <xsl:text>:</xsl:text>
        </xsl:if>
        <xsl:value-of select="@cat|@pt"/>
        </xhtml:p>
      </label>
      <xsl:apply-templates select="node"/>
    </node>
  </xsl:template>

  <!-- Leaf nodes -->
  <xsl:template match="node">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
        <xsl:value-of select="@rel"/>
        <!-- SoNaR annotation for semantic roles -->
        <xsl:if test="@pb">
          <xsl:text>/</xsl:text>
          <xhtml:font color="purple">
            <xsl:value-of select="@pb" />
          </xhtml:font>
        </xsl:if>
        <xhtml:br/>
        <xsl:value-of select="@index"/>
        <xsl:if test = "@index and (@cat|@pt|@pos)">
          <xsl:text>:</xsl:text>
        </xsl:if>
          <xsl:choose>
            <xsl:when test="@pt">
              <xsl:value-of select="@pt"/>
            </xsl:when>
            <xsl:when test="@pos">
              <xsl:value-of select="@pos"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="@cat"/>
            </xsl:otherwise>
          </xsl:choose>
        <xsl:if test="@lemma or @root">
          <xhtml:br/>
          <xhtml:i>
            <xsl:choose>
              <xsl:when test="@lemma and not(@lemma='_')">
                <xsl:value-of select="@lemma"/>
              </xsl:when>
              <!-- In Lassy @lemma='_' indicates that @lemma equals @word. -->
              <xsl:when test="@lemma and @lemma='_'">
                <xsl:value-of select="@word"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="@root"/>
              </xsl:otherwise>
            </xsl:choose>
          </xhtml:i>
        </xsl:if>
        </xhtml:p>
      </label>
      <xsl:if test="@postag or pos">
        <tooltip>
          <xhtml:i>
          <xsl:choose>
            <xsl:when test="@postag">
              <xsl:value-of select="@postag"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="@pos"/>
            </xsl:otherwise>
          </xsl:choose>
          </xhtml:i>
        </tooltip>
      </xsl:if>
    </node>
  </xsl:template>
</xsl:stylesheet>


