/*
 * Recorder.h
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#ifndef RECORDER_H_
#define RECORDER_H_

#include "Metadata.h"
#include <unistd.h> //sleep
#include "Variables.h"
#include <math.h>
#include "bass.h"
#include "Timeout.h"
#include <signal.h>
#include "Player.h"

class Recorder {
	std::vector<int> errors;
	std::string hwplug;
	int pid;
	Metadata song;
	int songSleep;
	std::string prefix;
	std::string arecordLogfile;
	Player& player = Player::instance();
	int countVolumeLevels(std::string file);
	Recorder();
	Recorder(const Recorder&);
	Recorder & operator =(const Recorder &);
public:
	static Recorder& instance() {
		static Recorder _instance;
		return _instance;
	}
	~Recorder() {
	}
	int getSongErrors(unsigned int type);
	void incSongErrors(unsigned int type);
	void resetSongErrors(unsigned int type);
	void killArecord();
	bool checkOverrun();
	bool testPlaying();
	int getSongLength(std::string filename);
	bool checkSongLength(std::string filename);
	bool checkSong(std::string filename, bool logActive);
	int calcSleepTime();
	bool sleepTimed();
	void startRecording(Metadata m, int num);
	int recordSong(Metadata m1, int num);
};

#endif /* RECORDER_H_ */
