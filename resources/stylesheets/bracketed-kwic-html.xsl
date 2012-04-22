<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns="http://www.w3.org/1999/xhtml"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:str="http://exslt.org/strings"
  xmlns:exsl="http://exslt.org/common"
  xmlns:set="http://exslt.org/sets"
  extension-element-prefixes="str exsl set">

  <xsl:strip-space elements="entry"/>

  <xsl:output method="xml" encoding="UTF-8"
    doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
    doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
    indent="yes"/>

  <xsl:template match="/">
    <html>
      <head>
        <title></title>
        <style type="text/css">
          <xsl:text disable-output-escaping="yes">
            <![CDATA[
            <!--
              body { background-color: #fff; color: #000; }
              body.show a.show,
              body.hide a.hide {
              display: none;
              visibility: hidden;
              }
              body.hide a.show,
              body.show a.hide {
              padding: .4em .6em;
              border: 1px solid #808080;
              text-decoration: none;
              color: black;
              }
              a.show:hover, a.hide:hover {
              background-color: #c0c0FF;
              }
              body.hide dt,
              body.hide div.f,
              body.hide a.hide {
              display: none;
              visibility: hidden;
              }
              body.hide a.show {
              display: inline;
              visibility: visible;
              }
              body.hide dd {
              margin-left: 0px;
              margin-bottom: .5em;
              }
              body.show a.show {
              display: none;
              visibility: hidden;
              }
              body.show a.hide {
              display: inline;
              visibility: visible;
              }
              dd span { color: #FFF; }
              .l1 { background-color: #7FCDBB; }
              .l2 { background-color: #41B6C4; }
              .l3 { background-color: #1D91C0; }
              .l4 { background-color: #225EA8; }
              .l5 { background-color: #0C2C84; }
              b { color: #80e; }
              table { border-bottom: 1px solid #ccc; }
            -->
            ]]>
          </xsl:text>
        </style>
        <xsl:text disable-output-escaping="yes">
          <![CDATA[
          <!--[if !IE]> -->
          ]]>
        </xsl:text>
        <style type="text/css">
          <xsl:text disable-output-escaping="yes">
            <![CDATA[
            td.l, td.r {
            white-space: nowrap;
            overflow: hidden;
            max-width: 100px;
            }
            td.l {
            direction: rtl;
            }
            ]]>
          </xsl:text>
        </style>
        <!-- <![endif]-->
        <script language="JavaScript" type="text/javascript">
          <xsl:text disable-output-escaping="yes">
            <![CDATA[
            <!--
              function show() {
              document.getElementById('main').className = 'show';
              }
              function hide() {
              document.getElementById('main').className = 'hide';
              }
              //-->
              ]]>
            </xsl:text>
          </script>
        </head>
        <body id="main" class="show">
          <p>
            <a href="javascript:show()" class="show">show filenames</a>
            <a href="javascript:hide()" class="hide">hide filenames</a>
          </p>
          <xsl:apply-templates />
        </body>
      </html>
    </xsl:template>

    <xsl:template match="entriesinfo">
      <table>
        <xsl:apply-templates />
      </table>
    </xsl:template>

    <xsl:template match="corpus">
      <tr>
        <td>Corpus:</td><td><xsl:apply-templates /></td>
      </tr>
    </xsl:template>

    <xsl:template match="filter">
      <tr>
        <td>Filter:</td><td><xsl:apply-templates /></td>
      </tr>
    </xsl:template>

    <xsl:template match="date">
      <tr>
        <td>Date:</td><td><xsl:apply-templates /></td>
      </tr>
    </xsl:template>

    <xsl:template match="sentence">
      <xsl:apply-templates select="bracket" />
    </xsl:template>

    <xsl:template match="entry">
      <div class="f"><xsl:value-of select="filename/text()" /></div>
      <table width="100%">
        <xsl:apply-templates select="sentence" />
      </table>
    </xsl:template>

    <xsl:template match="bracket">
      <xsl:variable name="sentTextNodes" select="exsl:node-set(ancestor::sentence//text())"/>
        <tr>
          <td width="40%" align="right" valign="top" class="l">
            <xsl:value-of select="str:concat(set:leading($sentTextNodes,./text()))" />
          </td>
          <td valign="bottom" class="r">
            <b>
              <xsl:value-of select="."/>
            </b>
            <xsl:value-of select="str:concat(set:trailing($sentTextNodes,descendant-or-self::*[last()]/text()))" />
          </td>
        </tr>
        <xsl:apply-templates select="bracket" />
    </xsl:template>

  </xsl:stylesheet>
