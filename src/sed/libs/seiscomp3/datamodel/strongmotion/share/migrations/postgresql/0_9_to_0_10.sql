
\echo Updating FilterParameter
ALTER TABLE FilterParameter
  ADD m_value_pdf_variable_content BYTEA,
  ADD m_value_pdf_probability_content BYTEA,
  ADD m_value_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating PeakMotion
ALTER TABLE PeakMotion
  ADD m_motion_pdf_variable_content BYTEA,
  ADD m_motion_pdf_probability_content BYTEA,
  ADD m_motion_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_atTime_pdf_variable_content BYTEA,
  ADD m_atTime_pdf_probability_content BYTEA,
  ADD m_atTime_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Record
ALTER TABLE Record
  ADD m_startTime_pdf_variable_content BYTEA,
  ADD m_startTime_pdf_probability_content BYTEA,
  ADD m_startTime_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating EventRecordReference
ALTER TABLE EventRecordReference
  ADD m_campbellDistance_pdf_variable_content BYTEA,
  ADD m_campbellDistance_pdf_probability_content BYTEA,
  ADD m_campbellDistance_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_ruptureToStationAzimuth_pdf_variable_content BYTEA,
  ADD m_ruptureToStationAzimuth_pdf_probability_content BYTEA,
  ADD m_ruptureToStationAzimuth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_ruptureAreaDistance_pdf_variable_content BYTEA,
  ADD m_ruptureAreaDistance_pdf_probability_content BYTEA,
  ADD m_ruptureAreaDistance_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_JoynerBooreDistance_pdf_variable_content BYTEA,
  ADD m_JoynerBooreDistance_pdf_probability_content BYTEA,
  ADD m_JoynerBooreDistance_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_closestFaultDistance_pdf_variable_content BYTEA,
  ADD m_closestFaultDistance_pdf_probability_content BYTEA,
  ADD m_closestFaultDistance_pdf_used BOOLEAN NOT NULL DEFAULT '0';

\echo Updating Rupture
ALTER TABLE Rupture
  ADD m_width_pdf_variable_content BYTEA,
  ADD m_width_pdf_probability_content BYTEA,
  ADD m_width_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_displacement_pdf_variable_content BYTEA,
  ADD m_displacement_pdf_probability_content BYTEA,
  ADD m_displacement_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_vt_to_vs_pdf_variable_content BYTEA,
  ADD m_vt_to_vs_pdf_probability_content BYTEA,
  ADD m_vt_to_vs_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_shallowAsperityDepth_pdf_variable_content BYTEA,
  ADD m_shallowAsperityDepth_pdf_probability_content BYTEA,
  ADD m_shallowAsperityDepth_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_slipVelocity_pdf_variable_content BYTEA,
  ADD m_slipVelocity_pdf_probability_content BYTEA,
  ADD m_slipVelocity_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_strike_pdf_variable_content BYTEA,
  ADD m_strike_pdf_probability_content BYTEA,
  ADD m_strike_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_length_pdf_variable_content BYTEA,
  ADD m_length_pdf_probability_content BYTEA,
  ADD m_length_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_area_pdf_variable_content BYTEA,
  ADD m_area_pdf_probability_content BYTEA,
  ADD m_area_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_ruptureVelocity_pdf_variable_content BYTEA,
  ADD m_ruptureVelocity_pdf_probability_content BYTEA,
  ADD m_ruptureVelocity_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_stressdrop_pdf_variable_content BYTEA,
  ADD m_stressdrop_pdf_probability_content BYTEA,
  ADD m_stressdrop_pdf_used BOOLEAN NOT NULL DEFAULT '0',
  ADD m_momentReleaseTop5km_pdf_variable_content BYTEA,
  ADD m_momentReleaseTop5km_pdf_probability_content BYTEA,
  ADD m_momentReleaseTop5km_pdf_used BOOLEAN NOT NULL DEFAULT '0';

