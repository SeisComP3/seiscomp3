  <proc name="stream100_loc10">
    <tree>
      <input name="Z" channel="Z" location="10" rate="100"/>
      <input name="N" channel="N" location="10" rate="100"/>
      <input name="E" channel="E" location="10" rate="100"/>
      <node filter="FS2D5" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
  </proc>
