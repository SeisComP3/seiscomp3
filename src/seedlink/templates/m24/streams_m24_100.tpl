  <proc name="m24_100">
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
      <input name="T" channel="T" location="" rate="1"/>
      <input name="B" channel="B" location="" rate="1"/>
      <input name="X" channel="X" location="" rate="1"/>
      <input name="Y" channel="Y" location="" rate="1"/>
      <node stream="AE"/>
    </tree>
  </proc>
