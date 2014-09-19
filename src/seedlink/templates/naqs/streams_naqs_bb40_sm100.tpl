  <proc name="naqs_bb40">
    <tree>
      <input name="BHZ" channel="Z" location="" rate="40"/>
      <input name="BHN" channel="N" location="" rate="40"/>
      <input name="BHE" channel="E" location="" rate="40"/>
      <node stream="SH"/>
      <node filter="F96C" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
  </proc>
  <proc name="naqs_sm100">
    <tree>
      <input name="ACZ" channel="Z" location="" rate="100"/>
      <input name="ACN" channel="N" location="" rate="100"/>
      <input name="ACE" channel="E" location="" rate="100"/>
      <node stream="SL"/>
    </tree>
  </proc>
  <proc name="naqs_bb40_sm100">
    <using proc="naqs_bb40"/>
    <using proc="naqs_sm100"/>
  </proc>

