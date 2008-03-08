<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
                xmlns="http://www.w3.org/1999/xhtml"
                xmlns:c="http://nicht.s8.xrea.com/2006/11/cereja"
                xmlns:h="http://www.w3.org/1999/xhtml"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                exclude-result-prefixes="xsl c h">

<xsl:output method="xml" encoding="UTF-8"
            doctype-public="-//W3C//DTD XHTML 1.1//EN"
            doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"
            indent="yes"/>

<xsl:strip-space elements="*"/>








<!-- BASIC TEMPLATES ================================================= {{{1 -->

<xsl:template match="/c:doc">
  <html xml:lang="ja">
    <head>
      <title><xsl:apply-templates select="c:head/c:title"/></title>
      <link rel="stylesheet" href="style.css" />
    </head>

    <body id="cereja">
      <div id="header">
        <h1><xsl:apply-templates select="c:head/c:title"/></h1>
        <xsl:comment>@@AD@@</xsl:comment>  <!-- for online manual version -->
        <p>Last modified: <xsl:apply-templates select="c:head/c:mtime"/></p>
      </div>

      <div id="body">
        <xsl:apply-templates select="c:body/node()"/>
      </div>

      <div id="footer">
        <address>Copyright <xsl:text disable-output-escaping="yes"><![CDATA[&#169;]]></xsl:text> 2006-2008 kana &lt;<a href="http://nicht.s8.xrea.com/">http://nicht.s8.xrea.com/</a>&gt;</address>
      </div>
    </body>
  </html>
</xsl:template>




<xsl:template match="c:section">
  <xsl:param name="level" select="1"/>

  <div class="section">
    <xsl:attribute name="id">
      <xsl:call-template name="id-of"/>
    </xsl:attribute>

    <xsl:apply-templates select="node()">
      <xsl:with-param name="level" select="$level + 1"/>
    </xsl:apply-templates>
  </div>
</xsl:template>

<xsl:template match="c:section/c:title">
  <xsl:param name="level"/>

  <xsl:if test="$level &lt; 1 or 6 &lt; $level">
    <xsl:message terminate="yes">
      <xsl:text>Nest-level of c:section is overflow: </xsl:text>
      <xsl:value-of select="$level"/>
    </xsl:message>
  </xsl:if>

  <xsl:element name="h{$level}">
    <xsl:if test="../@class">
      <xsl:attribute name="class">
        <xsl:value-of select="../@class"/>
      </xsl:attribute>
    </xsl:if>

    <xsl:apply-templates/>
  </xsl:element>
</xsl:template>




<xsl:template match="c:var">
  <xsl:choose>
    <xsl:when test="@name = 'VERSION'">
      <xsl:value-of select="$VERSION"/>
    </xsl:when>
    <xsl:when test="@name = 'MTIME'">
      <xsl:value-of select="$MTIME"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:message terminate="yes">
        <xsl:text>c:var: Unknown @name: </xsl:text>
        <xsl:value-of select="@name"/>
      </xsl:message>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="c:e">
  <xsl:text disable-output-escaping="yes">&amp;</xsl:text>
  <xsl:value-of select="@name"/>
  <xsl:text>;</xsl:text>
</xsl:template>




<xsl:template match="@id">
  <xsl:attribute name="id">
    <xsl:call-template name="id-of">
      <xsl:with-param name="target" select=".."/>
    </xsl:call-template>
  </xsl:attribute>
</xsl:template>

<xsl:template match="c:mtime|c:title">
  <xsl:apply-templates select="*|text()"/>
</xsl:template>

<xsl:template match="*|@*|text()">
  <xsl:if test="'c' = substring-before(name(), ':')">
    <xsl:message terminate="yes">
      <xsl:text>Unknown element: `</xsl:text>
      <xsl:value-of select="name()"/>
      <xsl:text>'</xsl:text>
    </xsl:message>
  </xsl:if>

  <xsl:copy>
    <xsl:apply-templates select="*|@*|text()"/>
  </xsl:copy>
</xsl:template>








<!-- LINK ============================================================ {{{1 -->

<xsl:template match="h:a">
  <a>
    <xsl:if test="@href">
      <xsl:attribute name="href">
        <xsl:choose>
          <xsl:when test="starts-with(@href, '/')">
            <xsl:call-template name="realize-link">
              <xsl:with-param name="href" select="@href"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="@href"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
    </xsl:if>

    <xsl:apply-templates select="*|text()"/>
  </a>
</xsl:template>

<xsl:template match="c:term">
  <xsl:variable name="term" select="string(.)"/>
  <a>
    <xsl:attribute name="href">
      <xsl:call-template name="link-of">
        <xsl:with-param name="target"
          select="//c:section[@id = 'terms']//*[string(.) = $term]"/>
      </xsl:call-template>
    </xsl:attribute>

    <xsl:value-of select="$term"/>
  </a>
</xsl:template>


<xsl:template name="realize-link">
  <xsl:param name="href"/>

  <xsl:value-of select="concat('#', translate($href, '/', '_'))"/>
</xsl:template>




<xsl:template name="id-of">
  <xsl:param name="target" select="."/>

  <xsl:if test="$target/ancestor::c:section[1]">
    <xsl:call-template name="id-of">
      <xsl:with-param name="target" select="$target/ancestor::c:section[1]"/>
    </xsl:call-template>
  </xsl:if>
  <xsl:value-of select="concat('_', $target/@id)"/>
</xsl:template>


<xsl:template name="link-of">
  <xsl:param name="target" select="."/>

  <xsl:text>#</xsl:text>
  <xsl:call-template name="id-of">
    <xsl:with-param name="target" select="$target"/>
  </xsl:call-template>
</xsl:template>








<!-- TABLE OF CONTENTS =============================================== {{{1 -->

<xsl:template match="c:contents">
  <div id="contents">
    <ol>
      <xsl:apply-templates select="/c:doc/c:body/c:section"
                           mode="generate-contents"/>
    </ol>
  </div>
</xsl:template>

<xsl:template match="c:section" mode="generate-contents">
  <li>
    <a>
      <xsl:attribute name="href">
        <xsl:call-template name="link-of"/>
      </xsl:attribute>
      <xsl:value-of select="c:title"/>
    </a>
    <xsl:if test="c:section">
      <ol>
        <xsl:apply-templates select="c:section" mode="generate-contents"/>
      </ol>
    </xsl:if>
  </li>
</xsl:template>








<!-- INDEX =========================================================== {{{1 -->

<xsl:template match="c:index">
  <div class="index">
    <ul>
      <xsl:for-each select="//*[./@class = current()/@type]">
        <xsl:sort select="."/>
        <li>
          <code>
            <a>
              <xsl:attribute name="href">
                <xsl:call-template name="index-link-of" />
              </xsl:attribute>
              <xsl:apply-templates select="*|text()"/>
            </a>
          </code>
        </li>
      </xsl:for-each>
    </ul>
  </div>
</xsl:template>

<xsl:template match="h:dt[@class]">
  <dt class="{@class}">
    <xsl:attribute name="id">
      <xsl:call-template name="index-id-of"/>
    </xsl:attribute>

    <code>
      <xsl:apply-templates select="*|text()"/>
    </code>
  </dt>
</xsl:template>




<xsl:template name="index-id-of">
  <xsl:call-template name="id-of"/>
  <xsl:value-of select="generate-id(.)"/>
</xsl:template>

<xsl:template name="index-link-of">
  <xsl:text>#</xsl:text>
  <xsl:call-template name="index-id-of"/>
</xsl:template>








</xsl:stylesheet>
<!-- vim: lisp foldmethod=marker foldlevel=0
-->
