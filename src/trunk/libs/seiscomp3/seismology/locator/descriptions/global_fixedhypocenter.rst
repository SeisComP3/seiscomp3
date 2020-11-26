Mining-related events are useful as ground truth events [#Bond]_
because the epicentre and depth can be constrained by physical inspection.
Unless a local seismograph network with accurate timing also locates the event,
and that information is available, the origin time must be estimated in order
for the event to be useful as ground truth. Existing location algorithms in
|scname|, including :ref:`Hypo71 <global_locsat>` and :ref:`LOCSAT <global_locsat>`
do not allow the determination of origin time given a set of arrivals and a
fixed hypocentre. There is a need, then, for a method of fixed hypocentre
origin time determination.

Objectives of this locator are:

* Inversion of arrival times of phase picks for source time fixing hypocenter location.
* Compatibility of the method of fixed-hypocentre origin time determination with
  the practise of the Comprehensive Test Ban Treaty Organization (CTBTO).
* Adoptation of a procedure which is compatible with the other locators supported by |scname|.
* Adoptation of a procedure which can reproduce results of legacy locators currently
  in use, such as GENLOC [#Pavl]_ and GRL, a
  grid-based locator developed at the Canadian Hazards Information Service (CHIS).

The implementation of this locator by :term:`gempa` was initiated and has received
initial funding by National Ressources Canada (NRCAN) [#NRCAN]_.


Methodology
===========

Given the measured arrival times :math:`t_i^k` of phase :math:`k` at
station :math:`i`, most methods of earthquake hypocentre location involve
minimization of the weighted squared sum of the residuals. That is,
minimization of:

.. math::

   |r_w|^2 = \sum_{i=1}^N {w_i^2 [ t_i^k - \tau - T_{model}^k(r_i,x) ]^2}

The residuals are computed by subtracting the expected arrival times
:math:`\tau - T_{model}^k(r_i,x)` based on a velocity model applied at the
coordinates of each station
:math:`r_i`.

Typically the weights can be a combination of the inverse of the
estimated pick uncertainty :math:`1/{\sigma}_i`, a distance term
:math:`d^k(\Delta)` and/or a residual weight term :math:`p(r_i)`.
Alternative weighting schemes can be applied but in this
implementation we weight by pick uncertainty alone: :math:`w_i=\frac{1}{{\sigma}_i}`.

In the general case, the model is a nonlinear function of its inputs, and there is no analytic solution for the origin time  and hypocenter  that minimize the norm. Typically, the solution is found iteratively, based on an initial guess for the origin time and hypocenter. This is the normal procedure for an earthquake without an a priori estimate of the hypocentral location.
When the hypocenter is in fact accurately constrained, the modeled travel time is a constant, so we can project each phase arrival back to an equivalent origin time

.. math ::

   \tau_i^k = t_i^k - T_{model}^k (r_i,x)

so that we only have to find  which minimizes:

.. math::

   |r_w|^2 = \sum_{i=1}^{N}w_i^2 [\tau_i^k - \tau]^2

The residuals are minimized by:


.. math::

   \tau = \frac{\sum_{i=1}^{N}w_i^2 (\tau_i^k)^2}{\sum_{i=1}^{N}w_i^2}.

Thus, the origin time is simply the weighted mean of the equivalent origin times, according to the velocity model, associated with the arrivals.

The standard error of this estimate is:

.. math::

   \sigma = \sqrt{\frac{\sum_{i=1}^{N}w_i^2 [\tau_i^k - \tau]^2}{\sum_{i=1}^{N}w_i^2}}.

The methodology for estimating error intervals and ellipses recommended for standard processing at the CTBTO [#Lee]_ is that of [#Jord]_ and is implemented in LOCSAT [#Bratt]_.
Uncertainty is represented by a set of points :math:`x_e` around the final estimate
:math:`x_f` satisfying:

.. math::

   \kappa_p^2 &= (x_e - x_f)^TC_m(x_e-x_f), \\
   \kappa_p^2 &= Ms^2F_p(M,K+N-M), \\
   s^2 &= \frac{Ks_K^2+|r_w|^2}{K+N-M}

where:

* :math:`C_m`: Covariance matrix, corresponding to the final hypocentre estimate :math:`x_f`.
* :math:`s^2`: Ratio of actual to assumed.
* :math:`\kappa_p^2`: The “confidence coefficient” at probability :math:`\rho`.
* :math:`F_p(m,n)`: Fisher-Snedecor quantile function (inverse cumulative F-distribution)
  for and degrees of freedom of numerator and denominator sum of squares, respectively, and probability.
* :math:`p`: Confidence level: the desired probability that the true epicentre should
  fall within the uncertainty bounds.
* :math:`N`: Sum of all arrival time, azimuth or slowness estimates. Here, only
  arrival times are considered for inversion.
* :math:`M`: Number of fitted parameters:

  * 3: error ellipsoid
  * 2: error ellipse
  * 1: depth or time error bounds.

  Here, :math:`M = 1` as we only invert for the time.

* :math:`s_K^2`: A prior estimate of the ratio of actual to assumed data variances; typically set to 1.
* :math:`K`: Number of degrees of freedom in prior estimate :math:`s_K^2`.
  :math:`K` can be configured by :confval:`FixedHypocenter.degreesOfFreedom`.
* :math:`r_w`: Vector of weighted residuals.


Although this formulation is complex it is useful it because allows the analyst to
balance a priori and a posteriori estimates of the ratio of actual to assumed
data variances.

The covariance matrix in the general case is computed from the weighted sensitivity
matrix :math:`A_w`, the row-weighted matrix of partial derivatives of arrival
time with respect to the solution coordinates.

.. math::

   C_m = A^T_wA_w

However, when origin time is the only coordinate, the partial derivatives with respect to origin time are unity, the weighted sensitivity matrix is simply a row vector of weights, and the time-time covariance
:math:`c_{tt}` is simply the sum of the squares of these weights.

.. math::

   c_{tt} = \sum_{i=1}^{N}w_i^2

It is recommended that fixed-hypocentre origin time confidence intervals be estimated using the method of [#Jord]_ for error ellipsoids, that is, that the time error bounds be represented using

.. math::

   \Delta t_p &= \sqrt{ \frac{\kappa_p^2}{c_{tt}} } \\
              &= \sqrt{ \frac{F_p(1,K+N-1)}{K+N-1} \frac{Ks_K^2 + \sum_{i=1}^{N}w_i^2 [\tau_i^k-\tau]^2}{\sum_{i=1}^{N}w_i^2}}.

In addition to recording arrival weights and residuals, distances and azimuths, and other details of origin quality, the details of a ground-truth-level (GT1) fixed-hypocentre origin time estimate are recorded as:

* origin.time = :math:`\tau`
* origin.time_errors.uncertainty = :math:`\Delta t_p`
* origin.time_errors.confidence_level = :math:`100p`
* origin.quality.standard_error = :math:`\sigma`
* origin.quality.ground_truth_level = GT1

For the sake of reproducibility, a comment is added to every new :term:`origin`
reporting :math:`K`, :math:`s_K` and :math:`\kappa_p`.

Application
===========

#. Configure the parameters in the section *FixedHypocenter* of the global configuration.
#. When using in :ref:`scolv` the FixedHypocenter locator can be chose right away
   from the available locators.

   .. figure:: media/scolv-fixedhypocenter.png
      :align: center
      :width: 18cm

      scolv Location tab with FixHypocenter selected for relocating.

#. Configure the module, e.g. :ref:`screloc` or :ref:`scolv`, which is to use FixedHypocenter:

   * set the locator type / interface: "FixedHypocenter"
   * if requested, set the profile as [interface]/[model], e.g.: LOCSAT/iasp91 or libtau/ak135

#. Run the module with FixedHypocenter

   Origins created by the FixedHypocenter locator can be identified by the methodID
   and the *confidence/description* comment of the origin paramters, e.g.: ::

      <origin publicID="Origin/20200102030459.123456.8222">
        ...
        <timeFixed>false</timeFixed>
        <epicenterFixed>true</epicenterFixed>
        <methodID>FixedHypocenter</methodID>
        <earthModelID>iasp91</earthModelID>
        ...
        <comment>
          <text>Confidence coefficient: K-weighted ($K$=8, $s_K$=1 s), $\kappa_p$ = 1.6, $n_{eff}$ = 5.0</text>
          <id>confidence/description</id>
        </comment>
        ...
      </origin>


References
==========

.. target-notes::

.. [#Bond] Bondár, I., & McLaughlin, K. L. (2009). A new ground truth data set for
   seismic studies. Seism. Res. Lett., 80(3), 465-472.
.. [#Bratt] Bratt, S. R., & Bache, T. C. (1988). Locating events with a sparse network of
   regional arrays. Bull. Seism. Soc. Am., 78(2), 780-798.
.. [#Bras] Le Bras, R., & Wuster, J. (2002). IDC Processing of Seismic, Hydroacoustic,
   and Infrasonic Data [IDC5.2.1Rev1]. Angewandte Wissenschaft, Software und
   Technologie GmbH.
.. [#Ever] Evernden, J. F. (1969). Precision of epicenters obtained by small numbers of
   world-wide stations. Bull. Seism. Soc. Am., 59(3), 1365-1398.
.. [#Flinn] Flinn, E. A. (1965). Confidence regions and error determinations for seismic
   event location. Rev. Geophys., 3(1), 157-185.
.. [#Jord] Jordan, T. H., & Sverdrup, K. A. (1981). Teleseismic location techniques and
   their application to earthquake clusters in the south-central Pacific. Bull.
   Seism. Soc. Am., 71(4), 1105-1130.
.. [#Klein] Klein, F. W. (2002). User's guide to HYPOINVERSE-2000, a Fortran program to
   solve for earthquake locations and magnitudes. US Geological Survey Open File
   Report 2002-171.
.. [#Lee] Lee, W. H., & Lahr, J. C. (1975). Hypo71 (revised): a computer program for
   determining local earthquake hypocentral parameters, magnitude, and first
   motion pattern of local earthquakes. Denver, CO: US Geol. Survey Open-file
   Report 75-311.
.. [#Pavl] Pavlis, G. L., Vernon, F., Harvey, D., & Quinlan, D. (2004). The generalized
   earthquake-location (GENLOC) package - An earthquake-location library.
   Computers & Geosciences, 30(9-10), 1079-1091.
.. [#NRCAN] National Ressources Canada (NRCAN), https://earthquakescanada.nrcan.gc.ca/index-en.php
