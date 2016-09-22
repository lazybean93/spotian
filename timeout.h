/*
 * timeout.h
 *
 *  Created on: 02.08.2016
 *      Author: oliver
 */

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <sys/time.h>

class Timeout {
	timeval start;
	timeval now;
	float timeout;

public:
	Timeout(float secs);
	bool inTime();
};


#endif /* TIMEOUT_H_ */
