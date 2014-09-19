  <proc name="stream100">
    <tree>
      <input name="Z" channel="Z" location="" rate="100"/>
      <input name="N" channel="N" location="" rate="100"/>
      <input name="E" channel="E" location="" rate="100"/>
      <node filter="FS2D5" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
  </proc>
