/*
 * Player.h
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <dbus/dbus.h>
#include "tools.h"
#include "variables.h"
#include <unistd.h> //sleep

class Player {
	bool playing;
	std::string prefix;
	bool controlPlayer(std::string action);
	Player();
	Player(const Player&);
	Player & operator =(const Player &);
public:
	static Player& instance() {
		static Player _instance;
		return _instance;
	}
	~Player() {
	}
	bool getPlaying();
	bool isPlaying();
	void setPlaying(bool set);
	bool pause();
	bool play();
	bool previous();
	bool next();
	bool pauseAtStart();
};

#endif /* PLAYER_H_ */
