/*
 * SpotifyController.h
 *
 *  Created on: 26.12.2016
 *      Author: oliver
 */

#ifndef SPOTIFYCONTROLLER_H_
#define SPOTIFYCONTROLLER_H_

#include <iostream>
#include "tools.h"
#include "Variables.h"
#include <sys/signal.h>
#include <unistd.h>

class SpotifyController {
	bool mainPage;
	int spotifyPid;
	SpotifyController();
	SpotifyController(const SpotifyController&);
	SpotifyController & operator =(const SpotifyController &);
	void killSpotify();
public:
	static SpotifyController& instance() {
		static SpotifyController _instance;
		return _instance;
	}
	~SpotifyController();
	void startSpotify();
	void setMainPage(bool mainPage);
	bool getMainPage();
	int getSpotifyPid();
	void setSpotifyPid(int pid);
};

#endif /* SPOTIFYCONTROLLER_H_ */
