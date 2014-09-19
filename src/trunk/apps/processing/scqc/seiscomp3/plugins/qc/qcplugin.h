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


#ifndef __SEISCOMP_QC_QCPLUGIN_H__
#define __SEISCOMP_QC_QCPLUGIN_H__

#include <seiscomp3/processing/application.h>
#include <seiscomp3/qc/qcprocessor.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/plugin.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/plugins/qc/api.h>
#include <seiscomp3/datamodel/waveformquality.h>

#include <queue>
#include <boost/thread.hpp>
#include <boost/signal.hpp>


namespace Seiscomp {
namespace Applications {
namespace Qc {


class QcMessenger;
class QcConfig;
class QcBuffer;
DEFINE_SMARTPOINTER(QcBuffer);

SC_QCPLUGIN_API DataModel::WaveformStreamID getWaveformID(const std::string &streamID);

class SC_QCPLUGIN_API QcApp : public Processing::Application {
	public:
		QcApp(int argc, char **argv) : Processing::Application(argc, argv) {};

		virtual bool exitRequested() const { return false; };
		virtual const QcConfig* qcConfig() const { return NULL; };
		virtual QcMessenger* qcMessenger() const { return NULL; };

		typedef boost::signal<void()> TimerSignal;
		virtual void addTimeout(const TimerSignal::slot_type& onTimeout) const {};
		virtual bool archiveMode() const { return false; };
                virtual std::string creatorID() const = 0;

 		TimerSignal doneSignal;
};

DEFINE_SMARTPOINTER(QcPlugin);

class QcPlugin : public Processing::QcProcessorObserver {
    DECLARE_SC_CLASS(QcPlugin);

public:
    
    QcPlugin();
    virtual ~QcPlugin();

    //! Initialize all needed ressources 
    virtual bool init(QcApp* app, QcConfig *cfg, std::string streamID);

    //! Called from the corresponding QcProcessor to propagate news
    virtual void update();
    
    //! Returns the plugin specific name given to plugin registry
    virtual std::string registeredName() const = 0;
    
    //! Returns the plugin specific parameter names
    virtual std::vector<std::string> parameterNames() const = 0;

    //! Returns the corresponding QcProcessor object
    Processing::QcProcessor* qcProcessor();
    
    //! Finish the work
    void done();
	
protected:
    void onTimeout();
    virtual void timeoutTask();
    
    virtual double mean(const QcBuffer* qcb) const;
    virtual double stdDev(const QcBuffer* qcb, double mean) const;
    
    virtual void generateNullReport() const;
    virtual void generateReport(const QcBuffer* reportBuffer) const;
    virtual void generateAlert(const QcBuffer* staBuffer, const QcBuffer* ltaBuffer) const;
    
    //! collect objects to be send to qcMessenger
    void pushObject(DataModel::Object* obj) const;

    //! pass the collected objects to qcMessenger
    //! false = data messages; true = notifier messages
    void sendObjects(bool notifier = false);
    
    void sendMessages(Core::Time &rectime);
    
    mutable std::queue<DataModel::ObjectPtr> _objects;
    std::string _name;
    std::vector<std::string> _parameterNames;
    std::string _streamID;
    QcApp* _app;
    QcMessenger* _qcMessenger;
    const QcConfig* _qcConfig;
    mutable QcBufferPtr _qcBuffer;
    Processing::QcProcessorPtr _qcProcessor;
    
    
private:
    mutable Core::Time _lastArchiveTime;
    mutable Core::Time _lastReportTime;
    mutable Core::Time _lastAlertTime;
    mutable bool _firstRecord;
    mutable Util::StopWatch _timer;
};

typedef std::vector<QcPluginPtr> QcPlugins;

DEFINE_INTERFACE_FACTORY(QcPlugin);

}
}
}

#define REGISTER_QCPLUGIN(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Applications::Qc::QcPlugin, Class> __##Class##InterfaceFactory__(Service)

#endif
