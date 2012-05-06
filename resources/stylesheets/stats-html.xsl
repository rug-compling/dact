<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:str="http://exslt.org/strings"
    xmlns:exsl="http://exslt.org/common"
    xmlns:set="http://exslt.org/sets"
    extension-element-prefixes="str exsl set">

    <xsl:output method="xml" encoding="UTF-8"
        doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
        doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
        indent="yes"/>

    <xsl:template match="/">
        <html>
            <head>
                <title>Statistics</title>
            </head>
            <body>
                <xsl:apply-templates />
            </body>
        </html>
    </xsl:template>

    <xsl:template match="statistics">
        <xsl:apply-templates select="statisticsinfo" />
        <p />
        <table border="1" cellspacing="0" cellpadding="4">
            <xsl:apply-templates select="statistic" />
        </table>
    </xsl:template>

    <xsl:template match="statisticsinfo">
        <table>
            <xsl:apply-templates />
        </table>
    </xsl:template>

    <xsl:template match="corpus|filter|attribute|date|variants|hits">
        <tr>
            <td>
                <xsl:value-of select="concat(translate(substring(local-name(), 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(local-name(), 2))" />
            </td>
            <td>
                <xsl:apply-templates />
            </td>
        </tr>
    </xsl:template>

    <xsl:template match="statistic">
        <tr>
                <xsl:apply-templates select="frequency"/>
                <xsl:apply-templates select="percentage"/>
                <xsl:apply-templates select="value"/>
        </tr>
    </xsl:template>

    <xsl:template match="frequency|value">
        <td>
            <xsl:apply-templates/>
        </td>
    </xsl:template>

    <xsl:template match="percentage">
        <td>
            <xsl:apply-templates/><xsl:text>%</xsl:text>
        </td>
    </xsl:template>
</xsl:stylesheet>
