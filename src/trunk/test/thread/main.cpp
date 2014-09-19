#include <seiscomp3/client/queue.h>
#include <seiscomp3/client/queue.ipp>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/datetime.h>

#include <boost/thread.hpp>
#include <iomanip>


Seiscomp::Client::ThreadedQueue<int> inputQueue(1);
Seiscomp::Client::ThreadedQueue<int> outputQueue(1);
Seiscomp::Client::ThreadedQueue<int> commitQueue(3);
Seiscomp::Core::Time refTime = Seiscomp::Core::Time::LocalTime();
Seiscomp::Core::Time cmtTime = refTime;


std::ostream &operator<<(std::ostream &os, const Seiscomp::Core::TimeSpan &ts) {
	os << std::setfill('0') << std::setw(2) << ts.seconds()
	   << "."
	   << std::setfill('0') << std::setw(3) << (ts.microseconds()/1000);
	return os;
}


void logTransfer(const char *prefix, int v, Seiscomp::Core::Time *last = NULL) {
	Seiscomp::Core::Time now = Seiscomp::Core::Time::LocalTime();
	Seiscomp::Core::TimeSpan diff = now-refTime;

	std::cout << diff << " " << prefix << v;

	if ( last ) {
		if ( last->valid() ) {
			diff = now-*last;
			std::cout << "  " << diff;
		}

		*last = now;
	}

	std::cout << std::endl;
}


void process() {
	while ( true ) {
		try {
			int v = inputQueue.pop();
			logTransfer("> - ", v);
			Seiscomp::Core::Time before = Seiscomp::Core::Time::LocalTime();
			if ( v & 1 )
				Seiscomp::Core::msleep(20+rand() % 15);
			else
				Seiscomp::Core::msleep(35+rand() % 10);
			std::cout << "P "
			          << (Seiscomp::Core::Time::LocalTime()-before)
			          << std::endl;
			//Seiscomp::Core::msleep(30+rand() % 15);
			outputQueue.push(v);
			logTransfer("< + ", v);
		}
		catch ( ... ) {
			break;
		}
	}
}


void process2() {
	while ( true ) {
		try {
			int v = commitQueue.pop();
			logTransfer("C   ", v, &cmtTime);
			Seiscomp::Core::msleep(40);
		}
		catch ( ... ) {
			break;
		}
	}
}


void commit(int v) {
	commitQueue.push(v);
	//logTransfer("C   ", v, &cmtTime);
}


int main(int argc, char **argv) {
	Seiscomp::Core::Time last = refTime;
	boost::thread processThread(process);
	boost::thread commitThread(process2);
	int cnt = 0;

	processThread.yield();

	while ( true ) {
		int v;
		bool hasOutput = false;

		// Flush output
		if ( outputQueue.canPop() ) {
			v = outputQueue.pop();
			hasOutput = true;
			logTransfer("< - ", v, &last);
		}

		if ( hasOutput) {
			commit(v);
			continue;
		}

		Seiscomp::Core::Time before = Seiscomp::Core::Time::LocalTime();
		if ( cnt & 1 )
			Seiscomp::Core::msleep(35+rand() % 10);
		else
			Seiscomp::Core::msleep(20+rand() % 15);
		std::cout << "V "
		          << (Seiscomp::Core::Time::LocalTime()-before)
		          << std::endl;

		if ( outputQueue.canPop() ) {
		//if ( !inputQueue.canPush() ) {
			v = outputQueue.pop();
			hasOutput = true;
			logTransfer("< - ", v, &last);
		}

		//Seiscomp::Core::Time before = Seiscomp::Core::Time::LocalTime();
		inputQueue.push(cnt);
		//std::cout << "P "
		//          << (Seiscomp::Core::Time::LocalTime()-before)
		//          << std::endl;
		logTransfer("> + ", cnt);
		++cnt;

		if ( hasOutput )
			commit(v);
	}

	return 0;
}
