/*
 * variables.h
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_

#define MAXLOADPAUSED 0.5
#define MAXLOADLOADING 3
#define WAITLOADTIMEOUTPAUSED 10
#define WAITLOADTIMEOUTPAUSECHECK 60
#define WAITLOADINGTIMEOUT 300

#define ADTIMEOUT 120

#include <iostream>
#include "tools.h"

class variables {
	std::string homedir;
	int spotifyPid;
	std::string display;
	variables() {
		homedir = std::string(getenv("HOME"));
		spotifyPid = -1;
		display = startDisplay();
		setenv("DISPLAY", display.c_str(), true);
	}
	variables(const variables&);
	variables & operator =(const variables &);
public:
	static variables& instance() {
		static variables _instance;
		return _instance;
	}
	~variables() {
	}
	std::string getHomedir() {
		return homedir;
	}
	std::string getDisplay() {
		return display;
	}
	int getSpotifyPid() {
		return spotifyPid;
	}
	void setSpotifyPid(int pid) {
		spotifyPid = pid;
	}

};

#endif /* VARIABLES_H_ */
