<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  extension-element-prefixes="str exsl">

  <xsl:output method="xml" encoding="UTF-8"/>

  <xsl:variable name="selectedNodes" select="//node[@active='1']"/>
  <xsl:variable name="words" select="str:tokenize(/*/sentence)"/>

  <!-- Result Tree Fragment, geen node-set -->
  <xsl:variable name="bpositions-rtf">
    <xsl:for-each select="$selectedNodes">
      <begin><xsl:value-of select="@begin + 1"/></begin>
      <end><xsl:value-of select="@end"/></end>
    </xsl:for-each>
  </xsl:variable>

  <!-- node-set maken -->
  <xsl:variable name="bpositions" select="exsl:node-set($bpositions-rtf)"/>

  <xsl:template match="/">
    <sentence>
      <xsl:for-each select="$words">
        <xsl:variable name="wordpos" select="position()"/>
        <xsl:for-each select="$bpositions/begin[.=$wordpos]">
          <xsl:text disable-output-escaping="yes"><![CDATA[<bracket>]]></xsl:text>
        </xsl:for-each>
        <xsl:value-of select="."/>
        <xsl:for-each select="$bpositions/end[.=$wordpos]">
          <xsl:text disable-output-escaping="yes"><![CDATA[</bracket>]]></xsl:text>
        </xsl:for-each>
        <xsl:if test="position() != last()">
          <xsl:text> </xsl:text>
        </xsl:if>
      </xsl:for-each>
    </sentence>
  </xsl:template>

</xsl:stylesheet>
