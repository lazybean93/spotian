/*
 * Recorder.cpp
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#include "Recorder.h"

int Recorder::countVolumeLevels(std::string file) {
	bool success;
	std::string content = readFile(file, &success);
	if (!success)
		return -1;

	std::vector<std::string> contentLines = split(content, "\n");
	int count = 0;
	for (unsigned int i = 0; i < contentLines.size(); i++) {
		if (contentLines.at(i).find("Max") == 0)
			count++;
	}
	return count;
}
int Recorder::getSongErrors(unsigned int type) {
	if (errors.size() < type + 1)
		return 0;
	return errors.at(type);
}
void Recorder::incSongErrors(unsigned int type) {
	while (type + 1 > errors.size())
		errors.push_back(0);
	errors.at(type)++;}
void Recorder::resetSongErrors(unsigned int type) {
	if (errors.size() > type)
		errors.at(type) = 0;
}
void Recorder::killArecord() {
	bool success = true;
	int arecordPid = getProcIdByName("arecord");
	while (success && arecordPid != -1) {
		kill(arecordPid, SIGTERM);
		readFile("/proc/" + numToString(arecordPid) + "/stat", &success);
		usleep(0.1 * 1000000);
	}
}
Recorder::Recorder() {
	hwplug = "0,1";
	prefix = "Recorder:\t";
	arecordLogfile = Variables::instance().getHomeDir() + "/arecordLog.txt";
}
bool Recorder::checkOverrun() {
	bool success;
	std::string content = readFile(arecordLogfile, &success) + "\n";
	if (!success || content.find("overrun!!!") != std::string::npos) {
		logline("Overrun found!", true);
		return false;
	}
	return true;
}
bool Recorder::testPlaying() {
	logline(prefix + "Soundcheck", true);
	int samples = 50;
	std::string file = "soundcheck.txt";

	//get maximal #number amount of volume levels in #timeoutsecs time
	execToString(
			"arecord -vvv plughw:" + hwplug + " 2> \"" + file
					+ "\" 1>/dev/null &");

	Timeout timeout = Timeout(10);
	while (countVolumeLevels(file) < samples && timeout.inTime())
		;

	//got volume levels, kill capturing
	killArecord();

	//Analyze volume levels
	std::string soundcheck = execToString(
			"cat \"" + file + "\" | grep 'Max' | grep 1000");
	std::vector<std::string> vulines = split(soundcheck, "%");
	int sum = 0;
	int cVulines = vulines.size();
	for (int i = 0; i < cVulines - 1; i++) {
		sum += atoi(vulines.at(i).substr(vulines.at(i).size() - 2, 2).c_str());
	}
	deleteFile(file);
	deleteFile("plughw:" + hwplug);

	if (vulines.size() != 0) {
		return sum / vulines.size() > 3;
	} else {
		logline(prefix + "vulines.size() equals 0!", true);
		return false;
	}
}
int Recorder::getSongLength(std::string filename) {
	return atoi(execToString("mp3info -p \"%S\" \"" + filename + "\"").c_str());
}
bool Recorder::checkSongLength(std::string filename) {
	//Check songlength
	int lengthIs = getSongLength(filename);
	int lengthShould = song.getLength();
	if (1.2 * lengthIs < lengthShould || lengthIs < lengthShould - 10) {
		logline("Song is to short!", true);
		logline(numToString(lengthIs) + " " + numToString(lengthShould), true);
		return false;
	}
	return true;
}
bool Recorder::checkSong(std::string filename, bool logActive) {

	//Read volumes each 20ms
	std::vector<float> vu20ms;
	float max = 0;
	BASS_Init(0, 44100, 0, 0, NULL);
	HSTREAM chan = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0,
	BASS_STREAM_DECODE);
	int err = BASS_ErrorGetCode();
	if (err != 0)
		logline(prefix + "Bass Error:" + numToString(err), true);
	if (chan) {
		QWORD len = BASS_ChannelSeconds2Bytes(chan, 0.0); // BASS_ChannelGetLevel takes 20ms from the channel
		char data[len];
		DWORD level, left, right;
		for (int i = 0; -1 != (level = BASS_ChannelGetLevel(chan)); i++) {
			left = LOWORD(level); // the left level
			right = HIWORD(level); // the right level
			float newvu = (float) ((int) left + (int) right) / (2 * 32768);
			vu20ms.push_back(newvu);
			if (newvu > max)
				max = newvu;
			BASS_ChannelGetData(chan, data, len); // get data away from the channel
		}
		BASS_StreamFree(chan);
	}
	BASS_Free();
	if (vu20ms.size() == 0) {
		logline(prefix + numToString(vu20ms.size()) + " VU values detected",
				true);
		return false;
	}
	//combine each 5 values together
	//std::vector<float> vu100ms;
	// for (int i = 0; i < (int) vu20ms.size(); i++) {
	// float newvu = 0;
	// int j;
	// for (j = i; j < (int) vu20ms.size() && j < i + 4; j++)
	// newvu += vu20ms.at(j);
	// vu100ms.push_back(100 * ((newvu / (j - i)) / max));
	// i = j;
	// }

	float cut = 0;
	float min = 999;
	float lowPercent = 0.5;
	float maxSec = 10;
	int start = std::max((int) vu20ms.size() * lowPercent,
			(int) vu20ms.size() - maxSec * 1000 / 20);

	for (int i = start; i < (int) vu20ms.size(); i++) {
		vu20ms.at(i) = 100 * (vu20ms.at(i) / max);
		if (vu20ms.at(i) < 1)
			cut = i;
		if (vu20ms.at(i) < min)
			min = vu20ms.at(i);
	}

	if (cut == 0) {
		logline(prefix + "Silence not Found, secound try", true);
		for (int i = start; i < (int) vu20ms.size(); i++) {
			if (vu20ms.at(i) < min + 0.1)
				cut = i;
		}
		if (cut > 0) {
			logline(
					prefix + "Secound try successfull: Silence found at "
							+ numToString((float) cut / vu20ms.size()), true);
		}
	}

	if (cut == 0)
		incSongErrors(0);
	else
		resetSongErrors(0);

	float factor = 0.02;
	logline(
			prefix + numToString((vu20ms.size() - 1) * factor) + "s -> "
					+ numToString((float) cut * factor) + "s: "
					+ numToString(vu20ms.at(cut)), true);

	if (getSongErrors(1) > 0)
		logline(prefix + "Addtime 1: " + numToString(getSongErrors(1)), true);

	if (vu20ms.size() - cut > (8 + getSongErrors(1)) * (1 / factor)) {
		logline(prefix + "Difference > 8s + Addtime", true);
		incSongErrors(1);
		return false;
	} else {
		resetSongErrors(1);

		//Cut out beginning of next song and store to tmp.mp3
		cut = factor * cut; //std::cout << cut << " " << (int) cut << std::endl;
		std::string cutstr = "mp3splt -g %\[@o\] \"" + filename + "\" 0.0 "
				+ numToString((int) cut / 60) + "."
				+ numToString(round(10 * (cut - ((int) cut / 60) * 60)) / 10)
				+ " -d " + Variables::instance().getHomeDir() + " -o tmp 2>&1;";
		execToString(cutstr);

		//Trim song silence and store to tmp_trimmed.mp3
		execToString(
				"mp3splt -r -g %\[@o\] " + Variables::instance().getHomeDir()
						+ "/tmp.mp3 2>&1;");

		//Compare Songlengths of tmp and tmp_trimmed version
		int lengthCut = getSongLength(
				Variables::instance().getHomeDir() + "/tmp.mp3");
		int lengthTrim = getSongLength(
				Variables::instance().getHomeDir() + "/tmp_trimmed.mp3");

		if (1.2 * lengthTrim < lengthCut || lengthTrim < lengthCut - 10) {
			logline("Trimmed Song is to short - take Cutted", true);
			execToString(
					"mv " + Variables::instance().getHomeDir() + "/tmp.mp3 "
							+ Variables::instance().getHomeDir()
							+ "/tmp_trimmed.mp3");
		}

		deleteFile(Variables::instance().getHomeDir() + "/tmp.mp3");
		logline(prefix + "Check for wrong cutting", true);
		if (vu20ms.size() - cut * (1 / factor) > 2 && getSongErrors(2) < 5) {
			incSongErrors(2);
			logline(prefix + "Errorcount: " + numToString(getSongErrors(2)),
					true);
			checkSong(Variables::instance().getHomeDir() + "/tmp_trimmed.mp3",
					logActive);

			std::string resetFilename = "mv "
					+ Variables::instance().getHomeDir()
					+ "/tmp_trimmed.mp3 \"";
			std::vector<std::string> splitfold = split(
					split(filename, ".mp3").at(0), "/");
			for (int i = 0; i < (int) splitfold.size(); i++)
				resetFilename += "/" + splitfold.at(i);
			resetFilename += ".mp3\" 2>&1";
			execToString(resetFilename);
			deleteFile(Variables::instance().getHomeDir() + "/tmp_trimmed.mp3");

			if (getSongErrors(2) >= 5) {
				return false;
			}
		}
		logline(prefix + "Cutting Correct", true);
		if (!checkSongLength(filename))
			return false;

		return true;
	}
	return false;
}
int Recorder::calcSleepTime() {
	if (getSongErrors(0) > 0)
		logline(prefix + "Addtime 0: " + numToString(getSongErrors(0)), true);

	int sleeptime = song.getLength() + getSongErrors(0);
	songSleep = sleeptime - 4;
	return sleeptime;
}
bool Recorder::sleepTimed() {
	//Check additional sleep time

	logline(prefix + "Sleeping " + numToString(songSleep), true);

	sleep(songSleep / 3);

	if (song != Metadata()) {
		return false;
	}

	sleep((songSleep / 3) * 2);

	return true;
}
void Recorder::startRecording(Metadata m, int num) {

	song = m;

	std::string cmd = "arecord -q -f cd -t raw -D hw:" + hwplug + " 2>"
			+ arecordLogfile + " | lame --add-id3v2 ";
	cmd += "--tt \"" + m.getTitle() + "\" "; //Title
	cmd += "--ta \"" + m.getArtist() + "\" "; //Artist
	cmd += "--tl \"" + m.getAlbum() + "\" "; //Album
	cmd += "--tn \"" + m.getTrackNumber() + "\" "; //TitleNumber
	cmd += "--tc \"" + m.getUrl() + "\" "; //Spotify Id as comment
	cmd += "-r -b 192 - \"" + Variables::instance().getDataDir() + "/" + m.getUrlMP3()
			+ "\" >/dev/null 2>&1 &";
	execToString(cmd);
}
int Recorder::recordSong(Metadata m1, int num) {
	startRecording(m1, num);

	Timeout songTimeout(calcSleepTime());
	Timeout adTimeout(0);

	if (!player.play() || !sleepTimed()) {
		logline("Something went wrong", true);
		killArecord();

		return 5002;
	}

	bool ad = false;
	Metadata m2 = m1;
	while (m1 == m2 && player.isPlaying()) {
		if (!ad) {
			if (!songTimeout.inTime()) {
				ad = true;
				logline("Ad found", true);
				killArecord();
				adTimeout = Timeout(ADTIMEOUT);
			}

		} else {
			if (!adTimeout.inTime()) {
				logline("Reached timeout", true);
				return 5003;
			}
		}
		sleep(2);
		m2 = Metadata();
	}
	if (!ad) {
		sleep(2);
		killArecord();
	}
	if (ad) return 1;
	return 0;
}
