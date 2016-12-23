/*
 * timeout.h
 *
 *  Created on: 02.08.2016
 *      Author: oliver
 */

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <sys/time.h>
#include <iostream>

class Timeout {
	timeval start;
	timeval now;
	float timeout;
	double runningTime;

public:
	Timeout(float secs);
	bool inTime();
	double getRunningTime();
	std::string getFormatTime();
};


#endif /* TIMEOUT_H_ */
