/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

%module(directors="1") DataModel
%{
#include "seiscomp3/datamodel/types.h"
#include "seiscomp3/datamodel/journaling_package.h"
#include "seiscomp3/datamodel/arclinklog_package.h"
#include "seiscomp3/datamodel/qualitycontrol_package.h"
#include "seiscomp3/datamodel/inventory_package.h"
#include "seiscomp3/datamodel/eventparameters_package.h"
#include "seiscomp3/datamodel/config_package.h"
#include "seiscomp3/datamodel/routing_package.h"
#include "seiscomp3/datamodel/timequantity.h"
#include "seiscomp3/datamodel/creationinfo.h"
#include "seiscomp3/datamodel/phase.h"
#include "seiscomp3/datamodel/comment.h"
#include "seiscomp3/datamodel/realquantity.h"
#include "seiscomp3/datamodel/integerquantity.h"
#include "seiscomp3/datamodel/axis.h"
#include "seiscomp3/datamodel/principalaxes.h"
#include "seiscomp3/datamodel/tensor.h"
#include "seiscomp3/datamodel/originquality.h"
#include "seiscomp3/datamodel/nodalplane.h"
#include "seiscomp3/datamodel/timewindow.h"
#include "seiscomp3/datamodel/waveformstreamid.h"
#include "seiscomp3/datamodel/sourcetimefunction.h"
#include "seiscomp3/datamodel/nodalplanes.h"
#include "seiscomp3/datamodel/confidenceellipsoid.h"
#include "seiscomp3/datamodel/originuncertainty.h"
#include "seiscomp3/datamodel/blob.h"
#include "seiscomp3/datamodel/realarray.h"
#include "seiscomp3/datamodel/complexarray.h"
#include "seiscomp3/datamodel/arclinkrequestsummary.h"
#include "seiscomp3/datamodel/databasereader.h"
#include "seiscomp3/datamodel/databasequery.h"
%}

%newobject Seiscomp::DataModel::DatabaseReader::loadJournaling;
%newobject Seiscomp::DataModel::DatabaseReader::loadArclinkLog;
%newobject Seiscomp::DataModel::DatabaseReader::loadQualityControl;
%newobject Seiscomp::DataModel::DatabaseReader::loadInventory;
%newobject Seiscomp::DataModel::DatabaseReader::loadEventParameters;
%newobject Seiscomp::DataModel::DatabaseReader::loadConfig;
%newobject Seiscomp::DataModel::DatabaseReader::loadRouting;

%include "datamodelbase.i"
%include "seiscomp3/datamodel/types.h"

optional(Seiscomp::DataModel::TimeQuantity);
optional(Seiscomp::DataModel::CreationInfo);
optional(Seiscomp::DataModel::Phase);
optional(Seiscomp::DataModel::RealQuantity);
optional(Seiscomp::DataModel::IntegerQuantity);
optional(Seiscomp::DataModel::Axis);
optional(Seiscomp::DataModel::PrincipalAxes);
optional(Seiscomp::DataModel::Tensor);
optional(Seiscomp::DataModel::OriginQuality);
optional(Seiscomp::DataModel::NodalPlane);
optional(Seiscomp::DataModel::TimeWindow);
optional(Seiscomp::DataModel::WaveformStreamID);
optional(Seiscomp::DataModel::SourceTimeFunction);
optional(Seiscomp::DataModel::NodalPlanes);
optional(Seiscomp::DataModel::ConfidenceEllipsoid);
optional(Seiscomp::DataModel::OriginUncertainty);
optional(Seiscomp::DataModel::Blob);
optional(Seiscomp::DataModel::RealArray);
optional(Seiscomp::DataModel::ComplexArray);
optional(Seiscomp::DataModel::ArclinkRequestSummary);
enum(Seiscomp::DataModel::OriginUncertaintyDescription);
optional_enum(Seiscomp::DataModel::OriginUncertaintyDescription);
enum(Seiscomp::DataModel::MomentTensorStatus);
optional_enum(Seiscomp::DataModel::MomentTensorStatus);
enum(Seiscomp::DataModel::OriginDepthType);
optional_enum(Seiscomp::DataModel::OriginDepthType);
enum(Seiscomp::DataModel::OriginType);
optional_enum(Seiscomp::DataModel::OriginType);
enum(Seiscomp::DataModel::EvaluationMode);
optional_enum(Seiscomp::DataModel::EvaluationMode);
enum(Seiscomp::DataModel::EvaluationStatus);
optional_enum(Seiscomp::DataModel::EvaluationStatus);
enum(Seiscomp::DataModel::PickOnset);
optional_enum(Seiscomp::DataModel::PickOnset);
enum(Seiscomp::DataModel::MomentTensorMethod);
optional_enum(Seiscomp::DataModel::MomentTensorMethod);
enum(Seiscomp::DataModel::DataUsedWaveType);
optional_enum(Seiscomp::DataModel::DataUsedWaveType);
enum(Seiscomp::DataModel::EventDescriptionType);
optional_enum(Seiscomp::DataModel::EventDescriptionType);
enum(Seiscomp::DataModel::EventType);
optional_enum(Seiscomp::DataModel::EventType);
enum(Seiscomp::DataModel::EventTypeCertainty);
optional_enum(Seiscomp::DataModel::EventTypeCertainty);
enum(Seiscomp::DataModel::SourceTimeFunctionType);
optional_enum(Seiscomp::DataModel::SourceTimeFunctionType);
enum(Seiscomp::DataModel::PickPolarity);
optional_enum(Seiscomp::DataModel::PickPolarity);
enum(Seiscomp::DataModel::StationGroupType);
optional_enum(Seiscomp::DataModel::StationGroupType);

// package base
%include "seiscomp3/datamodel/timequantity.h"
%include "seiscomp3/datamodel/creationinfo.h"
%include "seiscomp3/datamodel/phase.h"
%include "seiscomp3/datamodel/comment.h"
%include "seiscomp3/datamodel/realquantity.h"
%include "seiscomp3/datamodel/integerquantity.h"
%include "seiscomp3/datamodel/axis.h"
%include "seiscomp3/datamodel/principalaxes.h"
%include "seiscomp3/datamodel/tensor.h"
%include "seiscomp3/datamodel/originquality.h"
%include "seiscomp3/datamodel/nodalplane.h"
%include "seiscomp3/datamodel/timewindow.h"
%include "seiscomp3/datamodel/waveformstreamid.h"
%include "seiscomp3/datamodel/sourcetimefunction.h"
%include "seiscomp3/datamodel/nodalplanes.h"
%include "seiscomp3/datamodel/confidenceellipsoid.h"
%include "seiscomp3/datamodel/originuncertainty.h"
%include "seiscomp3/datamodel/blob.h"
%include "seiscomp3/datamodel/realarray.h"
%include "seiscomp3/datamodel/complexarray.h"
%include "seiscomp3/datamodel/arclinkrequestsummary.h"

// package Journaling
%include "seiscomp3/datamodel/journalentry.h"
%include "seiscomp3/datamodel/journaling.h"

// package ArclinkLog
%include "seiscomp3/datamodel/arclinkuser.h"
%include "seiscomp3/datamodel/arclinkstatusline.h"
%include "seiscomp3/datamodel/arclinkrequestline.h"
%include "seiscomp3/datamodel/arclinkrequest.h"
%include "seiscomp3/datamodel/arclinklog.h"

// package QualityControl
%include "seiscomp3/datamodel/qclog.h"
%include "seiscomp3/datamodel/waveformquality.h"
%include "seiscomp3/datamodel/outage.h"
%include "seiscomp3/datamodel/qualitycontrol.h"

// package Inventory
%include "seiscomp3/datamodel/stationreference.h"
%include "seiscomp3/datamodel/stationgroup.h"
%include "seiscomp3/datamodel/auxsource.h"
%include "seiscomp3/datamodel/auxdevice.h"
%include "seiscomp3/datamodel/sensorcalibration.h"
%include "seiscomp3/datamodel/sensor.h"
%include "seiscomp3/datamodel/responsepaz.h"
%include "seiscomp3/datamodel/responsepolynomial.h"
%include "seiscomp3/datamodel/responsefap.h"
%include "seiscomp3/datamodel/dataloggercalibration.h"
%include "seiscomp3/datamodel/decimation.h"
%include "seiscomp3/datamodel/datalogger.h"
%include "seiscomp3/datamodel/responsefir.h"
%include "seiscomp3/datamodel/auxstream.h"
%include "seiscomp3/datamodel/stream.h"
%include "seiscomp3/datamodel/sensorlocation.h"
%include "seiscomp3/datamodel/station.h"
%include "seiscomp3/datamodel/network.h"
%include "seiscomp3/datamodel/inventory.h"

// package EventParameters
%include "seiscomp3/datamodel/eventdescription.h"
%include "seiscomp3/datamodel/dataused.h"
%include "seiscomp3/datamodel/compositetime.h"
%include "seiscomp3/datamodel/pickreference.h"
%include "seiscomp3/datamodel/amplitudereference.h"
%include "seiscomp3/datamodel/reading.h"
%include "seiscomp3/datamodel/momenttensorcomponentcontribution.h"
%include "seiscomp3/datamodel/momenttensorstationcontribution.h"
%include "seiscomp3/datamodel/momenttensorphasesetting.h"
%include "seiscomp3/datamodel/momenttensor.h"
%include "seiscomp3/datamodel/focalmechanism.h"
%include "seiscomp3/datamodel/amplitude.h"
%include "seiscomp3/datamodel/stationmagnitudecontribution.h"
%include "seiscomp3/datamodel/magnitude.h"
%include "seiscomp3/datamodel/stationmagnitude.h"
%include "seiscomp3/datamodel/pick.h"
%include "seiscomp3/datamodel/originreference.h"
%include "seiscomp3/datamodel/focalmechanismreference.h"
%include "seiscomp3/datamodel/event.h"
%include "seiscomp3/datamodel/arrival.h"
%include "seiscomp3/datamodel/origin.h"
%include "seiscomp3/datamodel/eventparameters.h"

// package Config
%include "seiscomp3/datamodel/parameter.h"
%include "seiscomp3/datamodel/parameterset.h"
%include "seiscomp3/datamodel/setup.h"
%include "seiscomp3/datamodel/configstation.h"
%include "seiscomp3/datamodel/configmodule.h"
%include "seiscomp3/datamodel/config.h"

// package Routing
%include "seiscomp3/datamodel/routearclink.h"
%include "seiscomp3/datamodel/routeseedlink.h"
%include "seiscomp3/datamodel/route.h"
%include "seiscomp3/datamodel/access.h"
%include "seiscomp3/datamodel/routing.h"

// additional headers
%include "seiscomp3/datamodel/databasereader.h"
%include "seiscomp3/datamodel/databasequery.h"
