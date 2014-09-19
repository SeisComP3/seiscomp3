  <!-- High bandwidth version; use original streams when possible
       Reftek streams #1..4 (if configured) must be set to 100, 40,
       20, and 1 Hz

       Stream#  Rate   Deci   Gain   Ch# 1   2   3   4   5   6
          1      100      1     1       HHZ HHN HHE HNZ HNN HNE
          2       40      1     1       SHZ SHN SHE SNZ SNN SNE
          3       20      1     1       BHZ BHN BHE
          4        1      1     1       LHZ LHN LHE
                         10     4       VHZ VHN VHE             -->

  <proc name="reftek">
    <tree>
      <input name="0.0" channel="Z" location="" rate="100"/>
      <input name="0.1" channel="N" location="" rate="100"/>
      <input name="0.2" channel="E" location="" rate="100"/>
      <node stream="HH"/>
    </tree>
    <tree>
      <input name="0.3" channel="Z" location="" rate="100"/>
      <input name="0.4" channel="N" location="" rate="100"/>
      <input name="0.5" channel="E" location="" rate="100"/>
      <node stream="HN"/>
    </tree>
    <tree>
      <input name="1.0" channel="Z" location="" rate="40"/>
      <input name="1.1" channel="N" location="" rate="40"/>
      <input name="1.2" channel="E" location="" rate="40"/>
      <node stream="SH"/>
    </tree>
    <tree>
      <input name="1.3" channel="Z" location="" rate="40"/>
      <input name="1.4" channel="N" location="" rate="40"/>
      <input name="1.5" channel="E" location="" rate="40"/>
      <node stream="SN"/>
    </tree>
    <tree>
      <input name="2.0" channel="Z" location="" rate="20"/>
      <input name="2.1" channel="N" location="" rate="20"/>
      <input name="2.2" channel="E" location="" rate="20"/>
      <node stream="BH"/>
    </tree>
    <tree>
      <input name="3.0" channel="Z" location="" rate="1"/>
      <input name="3.1" channel="N" location="" rate="1"/>
      <input name="3.2" channel="E" location="" rate="1"/>
      <node stream="LH">
        <node filter="VLP" stream="VH"/>
      </node>
    </tree>
  </proc>
