<?xml version="1.0"?>
<!-- **********************************************************************
 * (C) 2006 - GFZ-Potsdam
 *
 * Author: Andres Heinloo
 * Email: geofon_devel@gfz-potsdam.de
 * $Date$
 * $Revision$
 * $LastChangedBy$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ********************************************************************** -->

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:qml="http://www.quakeml.ethz.ch/core"
    xmlns:scs="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.1">

<xsl:output method="xml" indent="yes"/>
<xsl:variable name="root">quakeml</xsl:variable>
<xsl:variable name="ns">http://www.quakeml.ethz.ch/core</xsl:variable>

<xsl:key name="elements_parents"
    match="//scs:type"
    use="scs:element/@typeref"/>

<xsl:key name="attributes_parents"
    match="//scs:type"
    use="scs:attribute/@typeref"/>

<xsl:template match="scs:seiscomp-schema">
    <xsl:comment> Generated from Seiscomp Schema, do not edit </xsl:comment>
    <xs:schema targetNamespace="{$ns}" elementFormDefault="qualified" attributeFormDefault="unqualified">
        <xs:include schemaLocation="quakeml_types.xsd"/>
        <xsl:apply-templates select="scs:enum"/>
        <xsl:apply-templates select="scs:type"/>
        <xs:element name="{$root}">
            <xs:complexType>
                <xs:all>
                    <xsl:apply-templates select="scs:element[(not (@ns) and /scs:seiscomp-schema/@ns = $ns) or @ns = $ns]" mode="package"/>
                </xs:all>
            </xs:complexType>
        </xs:element>
    </xs:schema>
</xsl:template>

<xsl:template name="find_namespaces">
    <xsl:param name="type" select="@name"/>
    <xsl:for-each select="key('elements_parents', $type)">
        <xsl:for-each select="scs:element[@typeref=$type]">
            <xsl:choose>
                <xsl:when test="@ns">,<xsl:value-of select="@ns"/>,</xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="find_namespaces">
                        <xsl:with-param name="type" select="../@name"/>
                    </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:for-each>
    <xsl:for-each select="key('attributes_parents', $type)">
        <xsl:for-each select="scs:attribute[@typeref=$type]">
            <xsl:choose>
                <xsl:when test="@ns">,<xsl:value-of select="@ns"/>,</xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="find_namespaces">
                        <xsl:with-param name="type" select="../@name"/>
                    </xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:for-each>
    <xsl:for-each select="/scs:seiscomp-schema">
        <xsl:for-each select="scs:element[@typeref=$type]">
            <xsl:choose>
                <xsl:when test="@ns">,<xsl:value-of select="@ns"/>,</xsl:when>
                <xsl:otherwise>,<xsl:value-of select="/scs:seiscomp-schema/@ns"/>,</xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:for-each>
</xsl:template>

<xsl:template match="scs:enum">
    <xsl:variable name="namespaces">
        <xsl:call-template name="find_namespaces"/>
    </xsl:variable>
    <xsl:if test="contains($namespaces, concat(',', $ns, ','))">
        <xs:simpleType name="{@name}">
            <xs:restriction base="xs:string">
                <xsl:apply-templates select="scs:option"/>
            </xs:restriction>
        </xs:simpleType>
    </xsl:if>
</xsl:template>
 
<xsl:template match="scs:option">
    <xs:enumeration value="{@value}"/>
</xsl:template>

<xsl:template match="scs:type">
    <xsl:variable name="namespaces">
        <xsl:call-template name="find_namespaces"/>
    </xsl:variable>
    <xsl:if test="contains($namespaces, concat(',', $ns, ','))">
        <xsl:variable name="attribute" select="scs:attribute[not (@ns) or @ns = $ns]"/>
        <xsl:variable name="element" select="scs:element[not (@ns) or @ns = $ns]"/>
        <xsl:if test="$attribute[@xmltype = 'element']">
            <xs:group name="gr_{@name}">
                <xs:all>
                    <xsl:apply-templates select="$attribute[@xmltype = 'element']" mode="element"/>
                </xs:all>
            </xs:group>
        </xsl:if>
        <xs:complexType name="{@name}">
            <xsl:choose>
                <xsl:when test="$attribute[@xmltype = 'cdata']">
                    <xsl:apply-templates select="$attribute[@xmltype = 'cdata']" mode="cdata"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:choose>
                        <xsl:when test="($attribute[@xmltype = 'element']) and ($element)">
                            <xs:sequence>
                                <xs:group ref="gr_{@name}"/>
                                <xs:choice minOccurs="0" maxOccurs="unbounded">
                                    <xsl:apply-templates select="$element"/>
                                </xs:choice>
                            </xs:sequence>
                        </xsl:when>
                        <xsl:when test="$attribute[@xmltype = 'element']">
                            <xs:group ref="gr_{@name}"/>
                        </xsl:when>
                        <xsl:when test="$element">
                            <xs:choice minOccurs="0" maxOccurs="unbounded">
                                <xsl:apply-templates select="$element"/>
                            </xs:choice>
                        </xsl:when>
                    </xsl:choose>
                    <xsl:apply-templates select="$attribute[@xmltype = 'attribute']"/>
                </xsl:otherwise>
            </xsl:choose>
        </xs:complexType>
    </xsl:if>
</xsl:template>

<xsl:template match="scs:element" mode="package">
    <xs:element name="{@name}" type="qml:{@typeref}" minOccurs="0" maxOccurs="1"/>
</xsl:template>

<xsl:template match="scs:element">
    <xs:element name="{@name}" type="qml:{@typeref}"/>
</xsl:template>

<xsl:template match="scs:attribute" mode="cdata">
    <xs:simpleContent>
        <xs:extension>
            <xsl:attribute name="base">
                <xsl:choose>
                    <xsl:when test="@typeref">
                        <xsl:text>qml:</xsl:text>
                        <xsl:value-of select="@typeref"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="@xsdtype"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:apply-templates select="../scs:attribute[@xmltype = 'attribute']"/>
        </xs:extension>
    </xs:simpleContent>
</xsl:template>

<xsl:template match="scs:attribute" mode="element">
    <xs:element name="{@name}">
        <xsl:attribute name="type">
            <xsl:choose>
                <xsl:when test="@typeref">
                    <xsl:text>qml:</xsl:text>
                    <xsl:value-of select="@typeref"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="@xsdtype"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:attribute>
        <xsl:attribute name="minOccurs">
            <xsl:choose>
                <xsl:when test="@optional = 'true'">0</xsl:when>
                <xsl:otherwise>1</xsl:otherwise>
            </xsl:choose>
        </xsl:attribute>
        <xsl:attribute name="maxOccurs">1</xsl:attribute>
    </xs:element>
</xsl:template>
            
<xsl:template match="scs:attribute">
    <xs:attribute name="{@name}">
        <xsl:attribute name="type">
            <xsl:choose>
                <xsl:when test="@typeref">
                    <xsl:text>qml:</xsl:text>
                    <xsl:value-of select="@typeref"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="@xsdtype"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:attribute>
        <xsl:if test="@optional = 'false'">
            <xsl:attribute name="use">required</xsl:attribute>
        </xsl:if>
    </xs:attribute>
</xsl:template>

</xsl:transform>

