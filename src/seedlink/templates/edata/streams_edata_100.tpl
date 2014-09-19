  <proc name="edata_100">
    <tree>
      <input name="Z" channel="Z" location="" rate="100"/>
      <input name="N" channel="N" location="" rate="100"/>
      <input name="E" channel="E" location="" rate="100"/>
      <node stream="HH"/>

<!--  Uncomment this to enable 50Hz SH? streams   -->
<!--                                              -->
<!--  <node filter="F96C" stream="SH"/>           -->

      <node filter="FS2D5" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
    <tree>
      <input name="Z1" channel="Z" location="" rate="100"/>
      <input name="N1" channel="N" location="" rate="100"/>
      <input name="E1" channel="E" location="" rate="100"/>
      <node stream="HN"/>
    </tree>
    <tree>
      <input name="S1" channel="1" location="" rate="1"/>
      <input name="S2" channel="2" location="" rate="1"/>
      <input name="S3" channel="3" location="" rate="1"/>
      <input name="S4" channel="4" location="" rate="1"/>
      <input name="S5" channel="5" location="" rate="1"/>
      <input name="S6" channel="6" location="" rate="1"/>
      <input name="S7" channel="7" location="" rate="1"/>
      <input name="S8" channel="8" location="" rate="1"/>
      <input name="PLL" channel="P" location="" rate="1"/>
      <node stream="AE"/>
    </tree>
  </proc>
