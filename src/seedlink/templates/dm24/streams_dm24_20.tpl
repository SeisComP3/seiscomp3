  <proc name="dm24_20">
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
  </proc>
