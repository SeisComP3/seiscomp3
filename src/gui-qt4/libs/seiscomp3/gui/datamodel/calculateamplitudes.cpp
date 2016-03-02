/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT ComputeAmplitudes

#include <seiscomp3/logging/log.h>
#include <seiscomp3/gui/datamodel/calculateamplitudes.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/magnitudes.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/utils/misc.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/mean.h>

#include <QProgressBar>
#include <QHeaderView>

#include <boost/bind.hpp>


using namespace std;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Processing;


namespace Seiscomp {

namespace Gui {

namespace {


string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


WaveformStreamID setWaveformIDComponent(const WaveformStreamID& id, char component) {
	return WaveformStreamID(id.networkCode(), id.stationCode(), id.locationCode(),
                            id.channelCode().substr(0,2) + component, id.resourceURI());
}


bool hasHigherPriority(AmplitudePtr candidate, AmplitudePtr reference) {
	return false;
}


}



CalculateAmplitudes::CalculateAmplitudes(Origin *origin,
                                         QWidget * parent, Qt::WindowFlags f)
 : QDialog(parent, f)
{
	_ui.setupUi(this);
	QFont font = _ui.source->font();
	font.setUnderline(true);
	_ui.source->setFont(font);
	_ui.table->horizontalHeader()->setStretchLastSection(true);

	_origin = origin;
	_thread = NULL;
	_query = NULL;
	_recomputeAmplitudes = false;
	_computeSilently = false;

	_externalAmplitudeCache = NULL;

	connect(_ui.comboFilterState, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(filterStateChanged(int)));

	connect(_ui.comboFilterType, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(filterTypeChanged(int)));
}


CalculateAmplitudes::~CalculateAmplitudes() {
	closeAcquisition();
}


void CalculateAmplitudes::setOrigin(DataModel::Origin *origin) {
	_origin = origin;
}


void CalculateAmplitudes::setStreamURL(const string& streamURL) {
	closeAcquisition();

	_ui.source->setText(streamURL.c_str());

	_thread = new RecordStreamThread(streamURL);
	connect(_thread, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));
	connect(_thread, SIGNAL(finished()),
	        this, SLOT(finishedAcquisition()));
}


void CalculateAmplitudes::setDatabase(DatabaseQuery *q) {
	_query = q;
}


void CalculateAmplitudes::setRecomputeAmplitudes(bool e) {
	_recomputeAmplitudes = e;
}


void CalculateAmplitudes::setAmplitudeCache(PickAmplitudeMap *cache) {
	_externalAmplitudeCache = cache;
}


void CalculateAmplitudes::setAmplitudeTypes(const TypeSet &types) {
	_amplitudeTypes = types;
}


int CalculateAmplitudes::exec() {
	if ( !process() )
		return QDialog::Rejected;

	return QDialog::exec();
}


void CalculateAmplitudes::done(int r) {
	if ( !_processors.empty() ) {
		if ( r == Rejected ) {
			if ( QMessageBox::question(this, tr("Cancel"),
			                           tr("Do you really want to cancel?"),
			                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
				return;
		}
		else {
			if ( QMessageBox::question(this, tr("OK"),
			                           tr("There are pending records to compute missing amplitudes.\n"
			                              "Do you really want to cancel and use all available results?"),
			                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
				return;
		}
	}

	closeAcquisition();

	QDialog::done(r);
}


void CalculateAmplitudes::closeAcquisition() {
	if ( _thread ) {
		_thread->stop(true);
		delete _thread;
		_thread = NULL;
	}
}


void CalculateAmplitudes::checkPriority(const AmplitudeEntry &newAmp) {
	iterator_range itp;
	itp = _amplitudes.equal_range(newAmp.first->pickID());
	for ( iterator it = itp.first; it != itp.second; ++it ) {
		if ( it->second.first->type() != newAmp.first->type() ) continue;

		DataModel::EvaluationMode cm = DataModel::AUTOMATIC;
		DataModel::EvaluationMode rm = DataModel::AUTOMATIC;

		try { cm = newAmp.first->evaluationMode(); }
		catch ( ... ) {}

		try { rm = it->second.first->evaluationMode(); }
		catch ( ... ) {}

		// Different evaluationMode: prefer MANUAL solutions
		if ( cm != rm ) {
			if ( cm == DataModel::MANUAL ) {
				it->second = newAmp;
				return;
			}
		}

		try {
			// Candidate is more recent than reference: prefer it
			if ( newAmp.first->creationInfo().creationTime() >
			     it->second.first->creationInfo().creationTime() ) {
				it->second = newAmp;
				return;
			}
		}
		catch ( ... ) {}
	}
}


bool CalculateAmplitudes::process() {
	//_ui.btnOK->setEnabled(false);

	_processors.clear();
	_ui.table->setRowCount(0);

	if ( !_origin || (_recomputeAmplitudes && !_thread) )
		return false;

	if ( _amplitudeTypes.empty() )
		return false;

	_timeWindow = Core::TimeWindow();

	/*
	TypeSet wantedAmpTypes;

	wantedAmpTypes.insert("MLv");
	wantedAmpTypes.insert("mb");
	wantedAmpTypes.insert("mB");
	wantedAmpTypes.insert("Mwp");

	try {
		vector<string> amps = SCApp->configGetStrings("amplitudes");
		wantedAmpTypes.clear();
		wantedAmpTypes.insert(amps.begin(), amps.end());
	}
	catch (...) {}
	*/

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( _thread )
		_thread->connect();

	typedef pair<PickCPtr, double> PickStreamEntry;

	// Typedef a pickmap that maps a streamcode to a pick
	typedef map<string, PickStreamEntry> PickStreamMap;

	// This map is needed to find the earliest P pick of
	// a certain stream
	PickStreamMap pickStreamMap;

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		Arrival *ar = _origin->arrival(i);

		double weight = 1.;
		try { weight = ar->weight(); } catch (Seiscomp::Core::ValueException) {}

		if ( Util::getShortPhaseName(ar->phase().code()) != 'P' || weight < 0.5 ) {
			continue;
		}

		Pick *pick = Pick::Find(ar->pickID());
		if ( !pick ) {
// 			cerr << " - Skipping arrival " << i << " -> no pick found" << endl;
			continue;
		}

		double dist = -1;
	
		try {
			dist = ar->distance();
		}
		catch ( Core::ValueError &e ) {
			try {
				Client::StationLocation loc;
				double azi1, azi2;

				loc = Client::Inventory::Instance()->stationLocation(pick->waveformID().networkCode(), pick->waveformID().stationCode(), pick->time().value());
				Math::Geo::delazi(loc.latitude, loc.longitude, _origin->latitude(), _origin->longitude(), &dist, &azi1, &azi2);
			}
			catch ( Core::GeneralException &e ) {}
		}


		DataModel::WaveformStreamID wfid = pick->waveformID();
		// Strip the component code because every AmplitudeProcessor
		// will use its own component to pick the amplitude on
		wfid.setChannelCode(wfid.channelCode().substr(0,2));

		string streamID = waveformIDToStdString(wfid);
		PickStreamEntry &e = pickStreamMap[streamID];

		// When there is already a pick registered for this stream which has
		// been picked earlier, ignore the current pick
		if ( e.first && e.first->time().value() < pick->time().value() )
			continue;

		e.first = pick;
		e.second = dist;
	}

	for ( PickStreamMap::iterator it = pickStreamMap.begin(); it != pickStreamMap.end(); ++it ) {
		PickCPtr pick = it->second.first;
		double dist = it->second.second;

		_ui.comboFilterType->clear();
		_ui.comboFilterType->addItem("- Any -");
		for ( TypeSet::iterator ita = _amplitudeTypes.begin(); ita != _amplitudeTypes.end(); ++ita )
			_ui.comboFilterType->addItem(ita->c_str());

		if ( _recomputeAmplitudes ) {
			for ( TypeSet::iterator ita = _amplitudeTypes.begin(); ita != _amplitudeTypes.end(); ++ita )
				addProcessor(*ita, pick.get(), dist);
		}
		else {
			string streamID = waveformIDToStdString(pick->waveformID());

			TypeSet usedTypes;

			if ( !_amplitudes.empty() ) {
				iterator_range itp;
				itp = _amplitudes.equal_range(pick->publicID());

				for ( iterator it = itp.first; it != itp.second; ) {
					AmplitudePtr amp = it->second.first;

					// The amplitude type is not one of the wanted types
					if ( _amplitudeTypes.find(amp->type()) == _amplitudeTypes.end() ) {
						++it;
						continue;
					}

					// Already has an amplitude of this type processed
					if ( usedTypes.find(amp->type()) != usedTypes.end() ) {
						++it;
						continue;
					}

					usedTypes.insert(amp->type());
		
					int row = addProcessingRow(waveformIDToStdString(amp->waveformID()), amp->type());
					setMessage(row, "read from cache");
					setValue(row, amp->amplitude().value());

					++it;
				}
			}

			bool foundAmplitudes = false;
			if ( _externalAmplitudeCache ) {
				iterator_range itp;
				itp = _externalAmplitudeCache->equal_range(pick->publicID());

				for ( iterator ita = itp.first; ita != itp.second; ++ita ) {
					AmplitudePtr amp = ita->second.first;

					// The amplitude type is not one of the wanted types
					if ( _amplitudeTypes.find(amp->type()) == _amplitudeTypes.end() )
						continue;

					// Already has an amplitude of this type processed
					if ( usedTypes.find(amp->type()) != usedTypes.end() ) {
						checkPriority(ita->second);
						continue;
					}

					usedTypes.insert(amp->type());

					int row = addProcessingRow(waveformIDToStdString(amp->waveformID()), amp->type());
					setMessage(row, "read from cache");
					setValue(row, amp->amplitude().value());

					_amplitudes.insert(PickAmplitudeMap::value_type(ita->first, ita->second));
				}
			}

			if ( _query && !foundAmplitudes ) {
				DatabaseIterator it = _query->getAmplitudesForPick(pick->publicID());
				for ( ; *it; ++it ) {
					AmplitudePtr amp = Amplitude::Cast(*it);
					if ( !amp ) continue;

					foundAmplitudes = true;

					// The amplitude type is not one of the wanted types
					if ( _amplitudeTypes.find(amp->type()) == _amplitudeTypes.end() ) continue;

					// Already has an amplitude of this type processed
					if ( usedTypes.find(amp->type()) != usedTypes.end() ) {
						checkPriority(AmplitudeEntry(amp, false));
						continue;
					}
	
					usedTypes.insert(amp->type());
		
					int row = addProcessingRow(waveformIDToStdString(amp->waveformID()), amp->type());
					setMessage(row, "read from database");
					setValue(row, amp->amplitude().value());
					_amplitudes.insert(PickAmplitudeMap::value_type(pick->publicID(), AmplitudeEntry(amp, false)));
				}
			}

			if ( !foundAmplitudes ) {
				EventParameters *ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
				if ( ep ) {
					for ( size_t i = 0; i < ep->amplitudeCount(); ++i ) {
						Amplitude *amp = ep->amplitude(i);
						if ( amp->pickID() != pick->publicID() ) continue;

						// The amplitude type is not one of the wanted types
						if ( _amplitudeTypes.find(amp->type()) == _amplitudeTypes.end() ) continue;

						// Already has an amplitude of this type processed
						if ( usedTypes.find(amp->type()) != usedTypes.end() ) {
							checkPriority(AmplitudeEntry(amp, false));
							continue;
						}
				
						usedTypes.insert(amp->type());
			
						int row = addProcessingRow(waveformIDToStdString(amp->waveformID()), amp->type());
						setMessage(row, "read from memory");
						setValue(row, amp->amplitude().value());
						_amplitudes.insert(PickAmplitudeMap::value_type(pick->publicID(), AmplitudeEntry(amp, false)));
					}
				}
			}

			TypeSet remainingTypes;
			set_difference(_amplitudeTypes.begin(), _amplitudeTypes.end(),
			               usedTypes.begin(), usedTypes.end(),
			               inserter(remainingTypes, remainingTypes.begin()));

			for ( TypeSet::iterator ita = remainingTypes.begin(); ita != remainingTypes.end(); ++ita ) {
				if ( _thread )
					addProcessor(*ita, pick.get(), dist);
				else {
					int row = addProcessingRow(streamID, *ita);
					setError(row, "missing");
				}
			}
		}
	}

	_ui.table->resizeColumnsToContents();
	_ui.table->resizeRowsToContents();

	if ( _thread && _timeWindow ) {
		_thread->setTimeWindow(_timeWindow);
		_thread->start();
	}

	QApplication::restoreOverrideCursor();

	return true;
}


void CalculateAmplitudes::addProcessor(
	const string &type,
	const DataModel::Pick *pick,
	double dist) {

	AmplitudeProcessorPtr proc = NULL;

	int row;

	proc = AmplitudeProcessorFactory::Create(type.c_str());
	if ( !proc ) {
		row = addProcessingRow(waveformIDToStdString(pick->waveformID()), type);
		setError(row, "no processor");
		return;
	}

	proc->setTrigger(pick->time().value());

	Util::KeyValues *keys = NULL;
	std::string stationID = pick->waveformID().networkCode() + "." +
	                        pick->waveformID().stationCode();
	ParameterMap::iterator it = _parameters.find(stationID);
	if ( it != _parameters.end() )
		keys = it->second.get();
	else if ( SCApp->configModule() != NULL ) {
		for ( size_t i = 0; i < SCApp->configModule()->configStationCount(); ++i ) {
			ConfigStation *station = SCApp->configModule()->configStation(i);

			if ( station->networkCode() != pick->waveformID().networkCode() ) continue;
			if ( station->stationCode() != pick->waveformID().stationCode() ) continue;

			Setup *setup = findSetup(station, SCApp->name());
			if ( setup ) {
				ParameterSet *ps = ParameterSet::Find(setup->parameterSetID());
				if ( !ps ) continue;

				Util::KeyValuesPtr keys_ = new Util::KeyValues;
				keys_->init(ps);
				_parameters[stationID] = keys_;
				keys = keys_.get();
			}
		}
	}

	proc->setReferencingPickID(pick->publicID());
	proc->setPublishFunction(boost::bind(&CalculateAmplitudes::emitAmplitude, this, _1, _2));

	switch ( proc->usedComponent() ) {
		case AmplitudeProcessor::Vertical:
			addProcessor(proc.get(), pick, WaveformProcessor::VerticalComponent);
			break;
		case AmplitudeProcessor::FirstHorizontal:
			addProcessor(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			break;
		case AmplitudeProcessor::SecondHorizontal:
			addProcessor(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		case AmplitudeProcessor::Horizontal:
			addProcessor(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			addProcessor(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		case AmplitudeProcessor::Any:
			addProcessor(proc.get(), pick, WaveformProcessor::VerticalComponent);
			addProcessor(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			addProcessor(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		default:
			row = addProcessingRow(waveformIDToStdString(pick->waveformID()), type);
			setError(row, "unsupported processor component");
			return;
	}
	
	// If initialization fails, abort
	if ( !proc->setup(
		Settings(
			SCApp->configModuleName(),
			pick->waveformID().networkCode(), pick->waveformID().stationCode(),
			pick->waveformID().locationCode(), pick->waveformID().channelCode().substr(0,2),
			&SCCoreApp->configuration(), keys)) ) {
		pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(proc.get());
		for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
			setError(row_it->second, QString("Setup failed"));
		return;
	}

	// Set depth hint
	try {
		proc->setHint(WaveformProcessor::Depth, _origin->depth());
		if ( proc->isFinished() ) {
			pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(proc.get());
			for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
				setError(row_it->second, QString("%1 (%2)").arg(proc->status().toString()).arg(proc->statusValue(), 0, 'f', 2));
			return;
		}
	}
	catch ( ... ) {}

	if ( dist >= 0 )
		proc->setHint(WaveformProcessor::Distance, dist);

	if ( proc->isFinished() ) {
		pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(proc.get());
		for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
			setError(row_it->second, QString("%1 (%2)").arg(proc->status().toString()).arg(proc->statusValue(), 0, 'f', 2));
		return;
	}

	proc->computeTimeWindow();

	switch ( proc->usedComponent() ) {
		case AmplitudeProcessor::Vertical:
			subscribeData(proc.get(), pick, WaveformProcessor::VerticalComponent);
			break;
		case AmplitudeProcessor::FirstHorizontal:
			subscribeData(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			break;
		case AmplitudeProcessor::SecondHorizontal:
			subscribeData(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		case AmplitudeProcessor::Horizontal:
			subscribeData(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			subscribeData(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		case AmplitudeProcessor::Any:
			subscribeData(proc.get(), pick, WaveformProcessor::VerticalComponent);
			subscribeData(proc.get(), pick, WaveformProcessor::FirstHorizontalComponent);
			subscribeData(proc.get(), pick, WaveformProcessor::SecondHorizontalComponent);
			break;
		default:
			return;
	}
}


void CalculateAmplitudes::addProcessor(Processing::AmplitudeProcessor *proc,
                                       const DataModel::Pick *pick,
                                       int c) {
	static const char *names[3] = {"vertical", "first horizontal", "second horizontal"};
	char component;
	ThreeComponents tc;

	try {
		tc = Client::Inventory::Instance()->getThreeComponents(pick);
	}
	catch ( ... ) {}

	WaveformStreamID cwid = pick->waveformID();

	if ( tc.comps[ThreeComponents::Component(c)] == NULL )
		component = '\0';
	else {
		cwid.setChannelCode(tc.comps[ThreeComponents::Component(c)]->code());
		component = *cwid.channelCode().rbegin();
	}

	std::string streamID = waveformIDToStdString(cwid);

	int row = addProcessingRow(streamID, proc->type());

	if ( component == '\0' ) {
		setError(row, QString("no %1 component found").arg(names[c]));
		return;
	}

	StreamMap::iterator it = _streams.find(streamID);
	if ( it != _streams.end() )
		proc->streamConfig((WaveformProcessor::Component)c) = *it->second;
	else {
		Processing::StreamPtr stream = new Processing::Stream;
		stream->init(cwid.networkCode(),
		             cwid.stationCode(),
		             cwid.locationCode(),
		             cwid.channelCode(),
		             pick->time().value());
		_streams[streamID] = stream;

		proc->streamConfig((WaveformProcessor::Component)c) = *stream;
	}

	if ( proc->streamConfig((WaveformProcessor::Component)c).gain == 0.0 ) {
		setError(row, "no gain found");
		return;
	}

	if ( proc->status() != WaveformProcessor::WaitingForData ) {
		setError(row, QString("%1 (%2)").arg(proc->status().toString()).arg(proc->statusValue(), 0, 'f', 2));
		return;
	}
	else
		setError(row, proc->status().toString());

	_rows.insert(TableRowMap::value_type(proc, row));
}


void CalculateAmplitudes::subscribeData(Processing::AmplitudeProcessor *proc,
                                        const DataModel::Pick *pick,
                                        int c) {
	if ( proc->streamConfig((WaveformProcessor::Component)c).code().empty() ) {
		SEISCOMP_WARNING("Empty channel code");
		return;
	}

	if ( proc->streamConfig((WaveformProcessor::Component)c).gain == 0.0 ) {
		SEISCOMP_WARNING("Invalid gain");
		return;
	}

	WaveformStreamID cwid = pick->waveformID();
	cwid.setChannelCode(proc->streamConfig((WaveformProcessor::Component)c).code());
	std::string streamID = waveformIDToStdString(cwid);

	pair<ProcessorMap::iterator, bool> handle = _processors.insert(ProcessorMap::value_type(streamID, ProcessorSlot()));
	if ( handle.second )
		_thread->addStream(cwid.networkCode(), cwid.stationCode(), cwid.locationCode(), cwid.channelCode());

	handle.first->second.push_back(proc);

	// Add processors timewindow to global acquisition timewindow
	_timeWindow = _timeWindow | proc->safetyTimeWindow();
}


int CalculateAmplitudes::addProcessingRow(const string &streamID, const string &type) {
	// Insert table row here
	int row = _ui.table->rowCount();

	_ui.table->insertRow(row);
	QTableWidgetItem *itemStream = new QTableWidgetItem(streamID.c_str());
	itemStream->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	QTableWidgetItem *itemType = new QTableWidgetItem(type.c_str());
	itemType->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	_ui.table->setItem(row, 0, itemStream);
	_ui.table->setItem(row, 1, itemType);

	return row;
}


CalculateAmplitudes::iterator CalculateAmplitudes::begin() {
	return _amplitudes.begin();
}


CalculateAmplitudes::iterator CalculateAmplitudes::end() {
	return _amplitudes.end();
}


CalculateAmplitudes::iterator_range
CalculateAmplitudes::pickAmplitudes(const std::string &pickID) {
	return _amplitudes.equal_range(pickID);
}


CalculateAmplitudes::iterator
CalculateAmplitudes::amplitude(const std::string &amplitudeID) {
	for ( iterator it = _amplitudes.begin(); it != _amplitudes.end(); ++it ) {
		if ( it->second.first->publicID() == amplitudeID )
			return it;
	}

	return _amplitudes.end();
}


bool CalculateAmplitudes::isNewlyCreated(const iterator &it) const {
	return it->second.second;
}


DataModel::AmplitudePtr
CalculateAmplitudes::amplitude(const iterator &it) const {
	return it->second.first;
}


void CalculateAmplitudes::setState(iterator it, bool e) {
	it->second.second = e;
}


void CalculateAmplitudes::setSilentMode(bool f) {
	_computeSilently = f;
}


void CalculateAmplitudes::emitAmplitude(const AmplitudeProcessor *proc,
                                        const Processing::AmplitudeProcessor::Result &res) {
	AmplitudePtr amp = Amplitude::Create();
	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	amp->setAmplitude(
		RealQuantity(
			res.amplitude.value, Core::None,
			res.amplitude.lowerUncertainty, res.amplitude.upperUncertainty,
			Core::None
		)
	);

	amp->setCreationInfo(ci);
	if ( res.period > 0 ) amp->setPeriod(RealQuantity(res.period));
	if ( res.snr >= 0 ) amp->setSnr(res.snr);
	amp->setType(proc->type());
	amp->setTimeWindow(
		TimeWindow(res.time.reference, res.time.begin, res.time.end)
	);

	if ( res.component >= 0 &&
	     res.component <= Processing::WaveformProcessor::SecondHorizontal )
		amp->setWaveformID(
			WaveformStreamID(
				res.record->networkCode(), res.record->stationCode(),
				res.record->locationCode(), proc->streamConfig((Processing::WaveformProcessor::Component)res.component).code(), ""
			)
		);
	else
		amp->setWaveformID(
			WaveformStreamID(
				res.record->networkCode(), res.record->stationCode(),
				res.record->locationCode(), res.record->channelCode().substr(0,2), ""
			)
		);

	amp->setPickID(proc->referencingPickID());

	TableRowMap::const_iterator it = _rows.find(proc);
	if ( it != _rows.end() )
		setValue(it->second, res.amplitude.value);

	_amplitudes.insert(PickAmplitudeMap::value_type(proc->referencingPickID(), AmplitudeEntry(amp, true)));
}


void CalculateAmplitudes::setValue(int row, double value) {
	QTableWidgetItem *itemValue = _ui.table->item(row, 2);
	if ( itemValue == NULL ) {
		itemValue = new QTableWidgetItem;
		itemValue->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		itemValue->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		_ui.table->setItem(row, 2, itemValue);
	}

	itemValue->setText(QString::number(value));
}


void CalculateAmplitudes::receivedRecord(Seiscomp::Record *rec) {
	Seiscomp::RecordPtr tmp(rec);

	ProcessorMap::iterator slot_it = _processors.find(rec->streamID());
	if ( slot_it == _processors.end() ) return;

	for ( ProcessorSlot::iterator it = slot_it->second.begin(); it != slot_it->second.end(); ) {
		(*it)->feed(rec);
		
		if ( (*it)->status() == WaveformProcessor::InProgress ) {
			pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(it->get());
			for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
				setProgress(row_it->second, (int)(*it)->statusValue());
			++it;
		}
		else if ( (*it)->status() == WaveformProcessor::Finished ) {
			pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(it->get());
			for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
				setProgress(row_it->second, 100);
			it = slot_it->second.erase(it);
		}
		else if ( (*it)->isFinished() ) {
			pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(it->get());
			for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
				setError(row_it->second, QString("%1 (%2)").arg((*it)->status().toString()).arg((*it)->statusValue(), 0, 'f', 2));
			it = slot_it->second.erase(it);
		}
		else
			++it;
	}

	if ( slot_it->second.empty() )
		_processors.erase(slot_it);
}


void CalculateAmplitudes::finishedAcquisition() {
	for ( ProcessorMap::iterator slot_it = _processors.begin(); slot_it != _processors.end(); ) {
		for ( ProcessorSlot::iterator it = slot_it->second.begin(); it != slot_it->second.end(); ) {
			if ( (*it)->status() == WaveformProcessor::InProgress ) {
				pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(it->get());
				for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
					setError(row_it->second, QString("incomplete data (%1% ok)").arg((*it)->statusValue(), 0, 'f', 2));
				(*it)->close();
				it = slot_it->second.erase(it);
			}
			else if ( (*it)->status() == WaveformProcessor::WaitingForData ) {
				pair<TableRowMap::iterator, TableRowMap::iterator> itp = _rows.equal_range(it->get());
				for ( TableRowMap::iterator row_it = itp.first; row_it != itp.second; ++row_it )
					setError(row_it->second, "no data");
				(*it)->close();
				it = slot_it->second.erase(it);
			}
			else if ( (*it)->isFinished() ) {
				(*it)->close();
				it = slot_it->second.erase(it);
			}
			else {
				(*it)->close();
				++it;
			}
		}

		if ( slot_it->second.empty() )
			_processors.erase(slot_it++);
		else
			++slot_it;
	}

	//_ui.btnOK->setEnabled(true);
	bool hasErrors = false;
	for ( int i = 0; i < _ui.table->rowCount(); ++i ) {
		if ( _ui.table->cellWidget(i, 3) != NULL ||
		     _ui.table->item(i, 3)->data(Qt::UserRole) == 1 )
			hasErrors = true;
	}

	if ( !hasErrors || _computeSilently )
		accept();
}


void CalculateAmplitudes::setError(int row, QString text) {
	// Remove progress bar if set
	_ui.table->setCellWidget(row, 3, NULL);

	QTableWidgetItem *itemState = new QTableWidgetItem(text);
	itemState->setData(Qt::TextColorRole, Qt::red);
	// Signal an error state
	itemState->setData(Qt::UserRole, 1);
	itemState->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	_ui.table->setItem(row, 3, itemState);
	filterView(row,1);
}


void CalculateAmplitudes::setMessage(int row, QString text) {
	_ui.table->setCellWidget(row, 3, NULL);

	QTableWidgetItem *itemState = new QTableWidgetItem(text);
	itemState->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	// Signal an non error state
	itemState->setData(Qt::UserRole, 0);
	_ui.table->setItem(row, 3, itemState);
	_ui.table->showRow(row);
	filterView(row,1);
}


void CalculateAmplitudes::setProgress(int row, int progress) {
	QProgressBar *progressBar = static_cast<QProgressBar*>(_ui.table->cellWidget(row, 3));
	_ui.table->setItem(row, 3, NULL);
	if ( !progressBar ) {
		progressBar = new QProgressBar(_ui.table);
		progressBar->setRange(0, 100);
		progressBar->setAlignment(Qt::AlignHCenter);
		QPalette pal = progressBar->palette();
		pal.setColor(QPalette::Highlight, Qt::darkGreen);
		progressBar->setPalette(pal);
	
		_ui.table->setCellWidget(row, 3, progressBar);
	}

	progressBar->setValue(progress);
	filterView(row, 1);
}


void CalculateAmplitudes::filterStateChanged(int) {
	filterView();
}


void CalculateAmplitudes::filterTypeChanged(int) {
	filterView();
}


void CalculateAmplitudes::filterView(int startRow, int cnt) {
	bool showSuccess = _ui.comboFilterState->currentIndex() == 1;
	bool showErrors = _ui.comboFilterState->currentIndex() == 2;
	bool showProgress = _ui.comboFilterState->currentIndex() == 3;

	QString type = _ui.comboFilterType->currentIndex() == 0?"":_ui.comboFilterType->currentText();

	int endRow;
	if ( cnt < 0 )
		endRow = _ui.table->rowCount();
	else
		endRow = startRow + cnt;

	for ( int i = startRow; i < endRow; ++i ) {
		bool hide = false;

		if ( showSuccess ) {
			QProgressBar *progressBar = static_cast<QProgressBar*>(_ui.table->cellWidget(i, 3));
			QTableWidgetItem *item = _ui.table->item(i, 3);
			if ( progressBar != NULL && progressBar->value() < 100 )
				hide = true;
			else if ( item && item->data(Qt::UserRole) == 1 )
				hide = true;
		}

		if ( showErrors ) {
			QTableWidgetItem *item = _ui.table->item(i, 3);
			if ( _ui.table->cellWidget(i, 3) != NULL ||
				 (item && item->data(Qt::UserRole) != 1) )
				hide = true;
		}

		if ( showProgress ) {
			QProgressBar *progressBar = static_cast<QProgressBar*>(_ui.table->cellWidget(i, 3));
			QTableWidgetItem *item = _ui.table->item(i, 3);
			if ( progressBar == NULL || progressBar->value() == 100 )
				hide = true;
			else if ( item && item->data(Qt::UserRole) == 1 )
				hide = true;
		}

		if ( !type.isEmpty() ) {
			if ( _ui.table->item(i, 1)->text() != type )
				hide = true;
		}

		if ( hide )
			_ui.table->hideRow(i);
		else
			_ui.table->showRow(i);
	}
}


}

}
