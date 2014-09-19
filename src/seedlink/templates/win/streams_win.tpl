  <proc name="win">
    <tree>
      <input name="Z" channel="Z" location="" rate="20"/>
      <input name="N" channel="N" location="" rate="20"/>
      <input name="E" channel="E" location="" rate="20"/>
      <node stream="BH"/>
      <node filter="F96C">
        <node filter="ULP" stream="LH">
          <node filter="VLP" stream="VH"/>
        </node>
      </node>
    </tree>
    <tree>
      <input name="AZ" channel="Z" location="" rate="100"/>
      <input name="AN" channel="N" location="" rate="100"/>
      <input name="AE" channel="E" location="" rate="100"/>
      <node stream="HL"/>
    </tree>
  </proc>
