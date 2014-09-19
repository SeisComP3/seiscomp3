  <!-- Reftek stream setup

       Low bandwidth version; derive everything from 40 Hz
       Reftek stream #2 must be set to 40 Hz

       Stream#  Rate   Deci   Gain   Ch# 1   2   3   4   5   6
          2      40       1     1       SHZ SHN SHE SNZ SNN SNE
                          2     1       BHZ BHN BHE
                         40     1       LHZ LHN LHE
                        400     4       VHZ VHN VHE             -->

  <proc name="reftek_6x40">
    <tree>
      <input name="1.0" channel="Z" location="" rate="40"/>
      <input name="1.1" channel="N" location="" rate="40"/>
      <input name="1.2" channel="E" location="" rate="40"/>
      <node stream="SH"/>
      <node filter="F96C" stream="BH">
        <node filter="F96C">
          <node filter="ULP" stream="LH">
            <node filter="VLP" stream="VH"/>
          </node>
        </node>
      </node>
    </tree>
    <tree>
      <input name="1.3" channel="Z" location="" rate="40"/>
      <input name="1.4" channel="N" location="" rate="40"/>
      <input name="1.5" channel="E" location="" rate="40"/>
      <node stream="SN"/>
    </tree>
  </proc>
