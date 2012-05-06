<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
    xmlns="urn:schemas-microsoft-com:office:spreadsheet"
    xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:str="http://exslt.org/strings"
    xmlns:exsl="http://exslt.org/common"
    xmlns:set="http://exslt.org/sets"
    extension-element-prefixes="str exsl set">

    <xsl:output method="xml" encoding="UTF-8" />

    <xsl:template match="/">
        <?mso-application progid="Excel.Sheet"?>
        <Workbook
            xmlns="urn:schemas-microsoft-com:office:spreadsheet"
            xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet">
            <Styles>
                <Style ss:ID="s01">
                    <NumberFormat ss:Format="0.0"/>
                </Style>
            </Styles>
            <xsl:apply-templates />
        </Workbook>
    </xsl:template>

    <xsl:template match="statistics">
        <xsl:variable name="nStatistics" select="count(statistic)" />

        <Worksheet ss:Name="Details">
            <Table>
                <xsl:apply-templates select="statistic" />
                <Row>
                    <Cell ss:Index="2">
                        <xsl:attribute name="ss:Formula">=SUM(R[-<xsl:value-of select="$nStatistics"/>]C:R[-1]C)</xsl:attribute>
                    </Cell>
                </Row>
            </Table>
        </Worksheet>

        <xsl:apply-templates select="statisticsinfo" />
    </xsl:template>

    <xsl:template match="statisticsinfo">
            <Worksheet ss:Name="Overview">
                <Table>
                    <xsl:apply-templates />
                </Table>
            </Worksheet>
    </xsl:template>

    <xsl:template match="corpus|filter|attribute|date">
        <Row>
            <Cell>
                <Data ss:Type="String">
                    <xsl:value-of select="concat(translate(substring(local-name(), 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(local-name(), 2))" />
                </Data>
            </Cell>
            <Cell>
                <Data ss:Type="String">
                    <xsl:apply-templates />
                </Data>
            </Cell>
        </Row>
    </xsl:template>

    <xsl:template match="variants|hits">
        <Row>
            <Cell>
                <Data ss:Type="String">
                    <xsl:value-of select="concat(translate(substring(local-name(), 1, 1), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'), substring(local-name(), 2))" />
                </Data>
            </Cell>
            <Cell>
                <Data ss:Type="Number">
                    <xsl:apply-templates />
                </Data>
            </Cell>
        </Row>
    </xsl:template>

    <xsl:template match="statistic">
        <Row>
            <xsl:apply-templates/>
        </Row>
    </xsl:template>

    <xsl:template match="value">
        <Cell>
            <Data ss:Type="String">
                <xsl:apply-templates />
            </Data>
        </Cell>
    </xsl:template>

    <xsl:template match="frequency">
        <Cell>
            <Data ss:Type="Number">
                <xsl:apply-templates />
            </Data>
        </Cell>
    </xsl:template>

    <xsl:template match="percentage">
        <xsl:variable name="parentPos" select="count(parent::*/preceding-sibling::statistic)" />
        <xsl:variable name="nStatistics" select="count(ancestor::statistics/statistic)" />
        <xsl:variable name="diff" select="$nStatistics - 1 - $parentPos" />
        <Cell ss:StyleID="s01">
            <xsl:attribute name="ss:Formula">=RC[-1]/SUM(R[-<xsl:value-of select="$parentPos"/>]C[-1]:R[<xsl:value-of select="$diff"/>]C[-1])*100</xsl:attribute>
        </Cell>
    </xsl:template>


</xsl:stylesheet>
