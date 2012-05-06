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

    <xsl:template match="statisticsinfo">
        <xsl:apply-templates />
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="corpus|filter|attribute|date">
        <xsl:value-of select="concat(translate(substring(local-name(), 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(local-name(), 2))" /><xsl:text>:&#x9;</xsl:text>
        <xsl:apply-templates />
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="variants|hits">
        <xsl:value-of select="concat(translate(substring(local-name(), 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(local-name(), 2))" /><xsl:text>:&#x9;</xsl:text>
        <xsl:apply-templates />
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="statistic">
        <xsl:apply-templates select="frequency" />
        <xsl:apply-templates select="percentage" />
        <xsl:apply-templates select="value" />
        <xsl:text>&#xA;</xsl:text>
    </xsl:template>

    <xsl:template match="percentage">
        <xsl:text>&#x9;</xsl:text>
        <xsl:apply-templates />
        <xsl:text>%</xsl:text>
    </xsl:template>

    <xsl:template match="frequency">
        <xsl:apply-templates />
    </xsl:template>

    <xsl:template match="value">
        <xsl:text>&#x9;</xsl:text>
        <xsl:apply-templates />
    </xsl:template>


</xsl:stylesheet>
