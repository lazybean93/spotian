#include "timeout.h"
#include "tools.h"

Timeout::Timeout(float secs) {
	timeout = secs;
	gettimeofday(&start, 0);
}

bool Timeout::inTime() {
	gettimeofday(&now, 0);
	runningTime = timevalDelta(now, start);
	if (runningTime >= timeout && timeout > 0)
		logline("running out of time", true);
	return (timevalDelta(now, start) < timeout);
}

double Timeout::getRunningTime(){
	inTime();
	return runningTime;
}

std::string Timeout::getFormatTime(){
	char buffer[8];
	inTime();
	int hour=(int)runningTime/3600;
	int second=(int)runningTime % 3600;
	int minute=second/60;
	second = second%60;
	sprintf(buffer,"%.2d:%.2d:%.2d",hour,minute,second);
	return std::string(buffer);
}
