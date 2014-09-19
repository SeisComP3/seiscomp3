  <proc name="stream40">
    <tree>
      <input name="Z" channel="Z" location="" rate="40"/>
      <input name="N" channel="N" location="" rate="40"/>
      <input name="E" channel="E" location="" rate="40"/>
      <node filter="F96C" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
  </proc>
