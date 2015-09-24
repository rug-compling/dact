<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xhtml="http://www.w3.org/1999/xhtml">

  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <!-- adapted from Jirka Kosek's page at 
       http://www.xml.com/pub/a/2004/09/08/tree.html -->

  <xsl:template match="/">
    <tree>
      <xsl:apply-templates select="/tueba_tree/node"/>
      <secedges>
        <xsl:apply-templates select="/tueba_tree//secEdge"/>
      </secedges>
    </tree>
  </xsl:template>

  <!-- Interior nodes -->
  <xsl:template match="node">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
        <xsl:value-of select="@func"/>
        <xhtml:br/>
        <xsl:value-of select="@cat|@pt"/>
        </xhtml:p>
      </label>
      <xsl:apply-templates select="node|ne|relation|word"/>
    </node>
  </xsl:template>

  <!-- Named entity nodes -->
  <xsl:template match="ne">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
          <xsl:value-of select="@type"/>
        </xhtml:p>
      </label>
      <xsl:apply-templates select="node|ne|relation|word"/>
    </node>
  </xsl:template>

  <!-- Leaf nodes -->
  <xsl:template match="word">
    <node>
      <xsl:copy-of select="@*"/>
      <label>
        <xhtml:p align="center">
        <xsl:value-of select="@func"/>
        <xhtml:br/>
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
        <xsl:if test="@form">
          <xhtml:br/>
          <xhtml:i>
            <xsl:value-of select="@form"/>
          </xhtml:i>
        </xsl:if>
        </xhtml:p>
      </label>
      <xsl:if test="@morph">
        <tooltip>
          <xhtml:i>
            <xsl:value-of select="@morph"/>
          </xhtml:i>
        </tooltip>
      </xsl:if>
    </node>
  </xsl:template>

  <xsl:template match="secEdge">
    <secedge>
      <xsl:attribute name="from">
        <xsl:value-of select="../self::*/@xml:id" />
      </xsl:attribute>
      <xsl:attribute name="to">
        <xsl:value-of select="@parent" />
      </xsl:attribute>
      <xsl:attribute name="cat">
        <xsl:value-of select="@cat" />
      </xsl:attribute>
    </secedge>
  </xsl:template>
</xsl:stylesheet>


