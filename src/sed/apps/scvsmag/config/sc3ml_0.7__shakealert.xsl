<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:scs="http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/0.7">
	<xsl:output method="xml" encoding="UTF-8" indent="yes"/>
	<xsl:strip-space elements="*"/>

	<!-- Starting point: Match the root node and select the one and only
		EventParameters node -->
	<xsl:template match="/scs:seiscomp">
	  <xsl:variable name="scsRoot" select="."/>
	  <xsl:for-each select="scs:EventParameters/scs:origin/scs:magnitude/scs:type[.='MVS']">
	    <xsl:sort select="../@publicID"/>
	    <xsl:choose>
	      <xsl:when test="position() = last()">
		<xsl:for-each select="../scs:comment/scs:id[.='update']">
		  <xsl:element name="event_message">
		    <xsl:call-template name="msgType">
		      <xsl:with-param name="updateno" select="../scs:text"/>
		    </xsl:call-template>
		    <xsl:element name="core_info">
		      <xsl:call-template name="eventID">
			<xsl:with-param name="id"
					select="$scsRoot/scs:EventParameters/scs:event/@publicID"/>
		      </xsl:call-template>
		      <xsl:call-template name="mvsmag">
			<xsl:with-param name="val"
					select="../../scs:magnitude/scs:value"/>
		      </xsl:call-template>
		      <xsl:call-template name="lh">
			<xsl:with-param name="val"
					select="../../scs:comment/scs:id[.='likelihood']/../scs:text"/>
		      </xsl:call-template>
		      <xsl:apply-templates select="$scsRoot/scs:EventParameters/scs:origin"/>
		    </xsl:element>
		  </xsl:element>
		</xsl:for-each>
	      </xsl:when>
	    </xsl:choose>
	  </xsl:for-each>
	</xsl:template>

	<!-- Delete elements -->
	<xsl:template match="scs:origin/scs:arrival"/>
	<xsl:template match="scs:origin/scs:creationInfo"/>
	<xsl:template match="scs:origin/scs:depthType"/>
	<xsl:template match="scs:origin/scs:quality"/>
	<xsl:template match="scs:origin/scs:uncertainty"/>
	<xsl:template match="scs:origin/scs:evaluationMode"/>
	<xsl:template match="scs:origin/scs:stationMagnitude"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:stationMagnitudeContribution"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:type"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:comment"/>
	<xsl:template match="scs:origin/scs:methodID"/>
	<xsl:template match="scs:origin/scs:earthModelID"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:creationInfo"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:stationCount"/>
	<xsl:template match="scs:origin/scs:magnitude/scs:magnitude"/>

        <xsl:template name="mvsmag">
	  <xsl:param name="val"/>
	    <xsl:element name="mag">
	      <xsl:attribute name="units">
		<xsl:value-of select="'Mw'"/>
	      </xsl:attribute>
	      <xsl:value-of select="$val"/>
	    </xsl:element>
	    <xsl:element name="mag_uncer">
	      <xsl:attribute name="units">
		<xsl:value-of select="'Mw'"/>
	      </xsl:attribute>
	      <xsl:value-of select="'0.5'"/>
	    </xsl:element>
	</xsl:template>

	<!--<xsl:template match="scs:origin/scs:magnitude/scs:comment"> 
		<xsl:for-each select="scs:id[.='likelihood']">
		</xsl:for-each>-->
	<xsl:template name="lh">
	  <xsl:param name="val"/>
	  <xsl:element name="likelihood">
	    <xsl:value-of select="$val"/>
	  </xsl:element>
	</xsl:template>

	<xsl:template match="scs:origin/scs:time">
		<xsl:element name="orig_time">
			<xsl:attribute name="units">
				<xsl:value-of select="'UTC'"/>
			</xsl:attribute>
			<!-- Chop off microsecond fractions as they are not understood by 
		the UserDisplay/DecisionModule -->
			<xsl:value-of select="concat(substring(./scs:value,1,24),'Z')"/>
		</xsl:element>
		<xsl:element name="orig_time_uncer">
			<xsl:attribute name="units">
				<xsl:value-of select="'sec'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:uncertainty"/>
		</xsl:element>
	</xsl:template>

	<xsl:template match="scs:origin/scs:latitude">
		<xsl:element name="lat">
			<xsl:attribute name="units">
				<xsl:value-of select="'deg'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:value"/>
		</xsl:element>
		<xsl:element name="lat_uncer">
			<xsl:attribute name="units">
				<xsl:value-of select="'deg'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:uncertainty"/>
		</xsl:element>
	</xsl:template>

	<xsl:template match="scs:origin/scs:longitude">
		<xsl:element name="lon">
			<xsl:attribute name="units">
				<xsl:value-of select="'deg'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:value"/>
		</xsl:element>
		<xsl:element name="lon_uncer">
			<xsl:attribute name="units">
				<xsl:value-of select="'deg'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:uncertainty"/>
		</xsl:element>
	</xsl:template>

	<xsl:template match="scs:origin/scs:depth">
		<xsl:element name="depth">
			<xsl:attribute name="units">
				<xsl:value-of select="'km'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:value"/>
		</xsl:element>
		<xsl:element name="depth_uncer">
			<xsl:attribute name="units">
				<xsl:value-of select="'km'"/>
			</xsl:attribute>
			<xsl:value-of select="./scs:uncertainty"/>
		</xsl:element>
	</xsl:template>

	<xsl:template name="eventID">
		<xsl:param name="id"/>
		<xsl:attribute name="id">
			<xsl:value-of select="$id"/>
		</xsl:attribute>
	</xsl:template>

	<xsl:template name="msgType">
		<xsl:param name="updateno"/>
		<xsl:attribute name="message_type">
			<xsl:choose>
				<xsl:when test="$updateno &lt; 1">
					<xsl:value-of select="'new'"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="'update'"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:attribute>
		<xsl:attribute name="orig_sys">
			<xsl:value-of select="'dm'"/>
		</xsl:attribute>
		<xsl:attribute name="version">
			<xsl:value-of select="$updateno"/>
		</xsl:attribute>
	</xsl:template>

	<!--Catch unmatched elements 
 <xsl:template match="*">
  <xsl:message terminate="no">
   WARNING: Unmatched element: <xsl:value-of select="name()"/>
  </xsl:message>
  <xsl:apply-templates/>
 </xsl:template>
 -->
</xsl:stylesheet>
