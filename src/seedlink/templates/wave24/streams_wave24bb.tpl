  <proc name="wave24bb">
    <tree>
      <input name="HHZ" channel="Z" location="" rate="100"/>
      <input name="HHN" channel="N" location="" rate="100"/>
      <input name="HHE" channel="E" location="" rate="100"/>
      <node stream="HH"/>
    </tree>
    <tree>
      <input name="BHZ" channel="Z" location="" rate="20"/>
      <input name="BHN" channel="N" location="" rate="20"/>
      <input name="BHE" channel="E" location="" rate="20"/>
      <node stream="BH"/>
    </tree>
  </proc>
