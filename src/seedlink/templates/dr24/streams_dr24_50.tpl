  <proc name="dr24_50">
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
    <tree>
      <input name="S0" channel="0" location="" rate="1/10"/>
      <input name="S1" channel="1" location="" rate="1/10"/>
      <input name="S2" channel="2" location="" rate="1/10"/>
      <input name="S3" channel="3" location="" rate="1/10"/>
      <input name="S4" channel="4" location="" rate="1/10"/>
      <input name="S5" channel="5" location="" rate="1/10"/>
      <input name="S6" channel="6" location="" rate="1/10"/>
      <input name="S7" channel="7" location="" rate="1/10"/>
      <input name="S8" channel="8" location="" rate="1/10"/>
      <input name="S9" channel="9" location="" rate="1/10"/>
      <node stream="AE"/>
    </tree>
  </proc>
