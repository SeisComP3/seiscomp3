  <proc name="generic_3x50">
    <tree>
      <input name="Z" channel="Z" location="" rate="50"/>
      <input name="N" channel="N" location="" rate="50"/>
      <input name="E" channel="E" location="" rate="50"/>
      <node stream="SH"/>
      <node filter="F96C" stream="BH">
        <node filter="FS2D5">
          <node filter="FS2D5" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
  </proc>
