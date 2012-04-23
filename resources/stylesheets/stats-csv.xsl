<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:str="http://exslt.org/strings"
    xmlns:exsl="http://exslt.org/common"
    xmlns:set="http://exslt.org/sets"
    extension-element-prefixes="str exsl set">

    <xsl:strip-space elements="statistics" />
    <xsl:strip-space elements="statisticsinfo" />

    <xsl:output method="text" encoding="UTF-8" />

    <xsl:template match="statisticsinfo" />

    <xsl:template match="statistic">
        <xsl:apply-templates select="value" /><xsl:text>,</xsl:text><xsl:apply-templates select="frequency" /><xsl:text>,</xsl:text><xsl:apply-templates select="percentage" />
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="value|frequency|percentage">
        <xsl:variable name="value" select="./text()" />
        <xsl:choose>
            <xsl:when test="contains($value, '&quot;')">
                <xsl:value-of select="concat('&quot;', replace($value, '&quot;', '&quot;&quot;'), '&quot;')"/>
            </xsl:when>
            <xsl:when test="contains($value, ',')">
                <xsl:value-of select="concat('&quot;', $value, '&quot;')"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$value"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
