The feature extraction as implemented at CTBTO IDC for single three-component
stations determines back azimuth (station to origin) and slowness, including the
uncertainties for both of these. In the IDC source code and database, the back
azimuth is referred to as only azimuth.

Algorithm
=========

The algorithm computes polarization attributes for a three-component station using
a modification to the Jurkevics (1988) [#Jurk]_ algorithm. Some of these attributes are
then used to determine detection azimuth (seazp = P-type azimuth in degrees),
detection slowness and azimuth/slowness uncertainties (inang1 = emergence (incidence)
angle and rect = rectilinearity).

A fixed noise window of 9.5 seconds ([-30s;-20.5s] with respect to trigger time)
and a signal window of 5.5 seconds ([-4s;1.5s] with respect to trigger time)
is used. The signal window is subdivided into intervals of 1.5s length which
overlap by 50%.

1. De-mean data according to mean of noise window
2. Apply cosine ramp to noise data and filter the entire data window
3. Rotate three components into ZNE space
4. Compute 3x3 covariance matrix for each interval
5. Extract eigenvalues and compute parameters including rectilinearity
6. Choose the result set with the largest rectilinearity


Picks
=====

In addition to the extracted back azimuth and slowness values the rectilinearity
is added as a comment to the resulting pick. The comment id is
``DFX:rectilinearity`` and the comment is the value in string representation.


References
==========

.. target-notes::

.. [#Jurk] Jurkevics, A. (1988). Polarization analysis of three-component array
   data. Bulletin of the Seismological Society of America, Vol. 78, No. 5,
   pp. 1725-1743, October 1988
