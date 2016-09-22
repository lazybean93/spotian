#include "timeout.h"
#include "tools.h"

Timeout::Timeout(float secs){
	timeout = secs;
	gettimeofday(&start, 0);
}

bool Timeout::inTime() {
		gettimeofday(&now, 0);
		if (timevalDelta(now, start) >= timeout)
			logline("running out of time", true);
		return (timevalDelta(now, start) < timeout);
	}
