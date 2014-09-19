scautopick searches for waveform anomalies in form of changes in amplitude.
It basically applies a robust sta/lta algorithm to the waveform streams which
have been filtered before with a Butterworth filter of third order with corner
frequencies of 0.7 and 2 Hz. Once the sta/lta ratio reached a specific value
(by default 3) for a particular stream, a pick is set to the time when this
threshold is exceeded. Once the ratio reaches a factor of 3, a pick is created
and the picker is set inactive. The picker is reactivated for this stream once
the sta/lta ratio falls to the value of 1.5.

The second task of scautopick is to calculate amplitudes for given magnitude
types where the time window starts at the pick time. For example mb is calculated
for a fixed time window of 30s after the pick, mB for time window of 90s, for
MLv a time window of 150s is estimated to make sure that S-arrivals are inside
this time window. The precalculated amplitudes are sent out and received by
the magnitude tool.

New Features
============

New features have been added to configure the picker. These are useful when we
would like to use a secondary picking module with different configuration or
add specific parameter for a single station.

global-case:

.. code-block:: sh

   module.trunk.global.detecStream = HH
   module.trunk.global.detecLocID = ""
   module.trunk.global.detecFilter = \
       "RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)"
   module.trunk.global.trigOn = 3
   module.trunk.global.trigOff = 1.5

In the above case all the channels will be picked on HH channel with same filter
and trigOn trigOff values.

station-case:

.. code-block:: sh

   module.trunk.CH.FUORN.detecStream = EH
   module.trunk.CH.FUORN.detecFilter = \
       "RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)"
   module.trunk.CH.FUORN.trigOn = 2
   module.trunk.CH.FUORN.trigOff = 1

in this case, regardless what has been defined in the trunk/key, in the profile and
in the module.trunk.global variable, for station FUORN we will pick on the EH
channels with this specific parameters.

We can also mix the two above cases, e.g.

.. code-block:: sh

   module.trunk.global.detecStream = HH
   module.trunk.global.detecLocID = ""
   module.trunk.global.detecFilter = \
       "RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)"
   module.trunk.global.trigOn = 3
   module.trunk.global.trigOff = 1.5
   module.trunk.CH.EMBD.detecStream = EH
   module.trunk.CH.ZUR.trigOn = 10

In this case we have a global config for the entire network and a specific stream
for EMBD, and a specific trigger threshold for ZUR.
