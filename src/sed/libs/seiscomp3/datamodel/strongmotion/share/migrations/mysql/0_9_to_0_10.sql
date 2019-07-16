
\echo Updating FilterParameter
ALTER TABLE FilterParameter
  ADD value_pdf_variable_content BLOB,
  ADD value_pdf_probability_content BLOB,
  ADD value_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

\echo Updating PeakMotion
ALTER TABLE PeakMotion
  ADD motion_pdf_variable_content BLOB,
  ADD motion_pdf_probability_content BLOB,
  ADD motion_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD atTime_pdf_variable_content BLOB,
  ADD atTime_pdf_probability_content BLOB,
  ADD atTime_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

\echo Updating Record
ALTER TABLE Record
  ADD startTime_pdf_variable_content BLOB,
  ADD startTime_pdf_probability_content BLOB,
  ADD startTime_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

\echo Updating EventRecordReference
ALTER TABLE EventRecordReference
  ADD campbellDistance_pdf_variable_content BLOB,
  ADD campbellDistance_pdf_probability_content BLOB,
  ADD campbellDistance_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD ruptureToStationAzimuth_pdf_variable_content BLOB,
  ADD ruptureToStationAzimuth_pdf_probability_content BLOB,
  ADD ruptureToStationAzimuth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD ruptureAreaDistance_pdf_variable_content BLOB,
  ADD ruptureAreaDistance_pdf_probability_content BLOB,
  ADD ruptureAreaDistance_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD JoynerBooreDistance_pdf_variable_content BLOB,
  ADD JoynerBooreDistance_pdf_probability_content BLOB,
  ADD JoynerBooreDistance_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD closestFaultDistance_pdf_variable_content BLOB,
  ADD closestFaultDistance_pdf_probability_content BLOB,
  ADD closestFaultDistance_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

\echo Updating Rupture
ALTER TABLE Rupture
  ADD width_pdf_variable_content BLOB,
  ADD width_pdf_probability_content BLOB,
  ADD width_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD displacement_pdf_variable_content BLOB,
  ADD displacement_pdf_probability_content BLOB,
  ADD displacement_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD vt_to_vs_pdf_variable_content BLOB,
  ADD vt_to_vs_pdf_probability_content BLOB,
  ADD vt_to_vs_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD shallowAsperityDepth_pdf_variable_content BLOB,
  ADD shallowAsperityDepth_pdf_probability_content BLOB,
  ADD shallowAsperityDepth_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD slipVelocity_pdf_variable_content BLOB,
  ADD slipVelocity_pdf_probability_content BLOB,
  ADD slipVelocity_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD strike_pdf_variable_content BLOB,
  ADD strike_pdf_probability_content BLOB,
  ADD strike_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD length_pdf_variable_content BLOB,
  ADD length_pdf_probability_content BLOB,
  ADD length_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD area_pdf_variable_content BLOB,
  ADD area_pdf_probability_content BLOB,
  ADD area_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD ruptureVelocity_pdf_variable_content BLOB,
  ADD ruptureVelocity_pdf_probability_content BLOB,
  ADD ruptureVelocity_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD stressdrop_pdf_variable_content BLOB,
  ADD stressdrop_pdf_probability_content BLOB,
  ADD stressdrop_pdf_used TINYINT(1) NOT NULL DEFAULT '0',
  ADD momentReleaseTop5km_pdf_variable_content BLOB,
  ADD momentReleaseTop5km_pdf_probability_content BLOB,
  ADD momentReleaseTop5km_pdf_used TINYINT(1) NOT NULL DEFAULT '0';

