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
#define WAITLOADINGTIMEOUT 60

#define ADTIMEOUT 120

#include <iostream>
#include "tools.h"

class variables {
	std::string homedir;
	std::string musicdir;
	std::string datadir;
	int spotifyPid;
	std::string display;
	std::vector<std::string> initPlaylists;
	std::vector<std::string> initPlaylistTracks;
	int initPlaylist = 1;
	std::string uri;
	std::string user;
	std::string pass;
	std::string tempLogFile;
	std::string logFile;
	bool loginLoggingActive;
	bool mainPage;
	variables() {
		mainPage = true;
		homedir = std::string(getenv("HOME"));
		musicdir = homedir+"/tmp";
		datadir = musicdir+"/data";


		mkdir(musicdir);
		mkdir(datadir);
		spotifyPid = -1;

		initPlaylists.push_back("spotify:album:2ppmPxlKzpaJy23sbe3vq8");
		initPlaylistTracks.push_back("spotify:track:3Z58ppU1Q7nwAfwCWCTRjv");

		initPlaylists.push_back("spotify:album:40lxWZ19hZgH0s71KGXOyo");
		initPlaylistTracks.push_back("spotify:track:4YuGhuEypxkqhd3QdewSuk");

		display = startDisplay();
		setenv("DISPLAY", display.c_str(), true);
		tempLogFile = getHomeDir() + "/spotifytemplog_" + getDisplay() + ".txt";
		logFile = getHomeDir() + "/spotifylog_" + getDisplay() + ".txt";
		loginLoggingActive = true;
	}
	variables(const variables&);
	variables & operator =(const variables &);
public:

	std::vector<std::string> prefix;
	static variables& instance() {
		static variables _instance;
		return _instance;
	}
	~variables() {
	}
	void setMainPage(bool mainPage){
		this->mainPage = mainPage;
	}
	bool getMainPage(){
		return mainPage;
	}
	std::string getHomeDir() {
		return homedir;
	}
	std::string getMusicDir() {
		return musicdir;
	}
	std::string getDataDir() {
		return datadir;
	}
	std::string getDisplay() {
		return display;
	}
	std::string getTestPlaylist() {
		initPlaylist = (initPlaylist+1)%initPlaylists.size();
		return initPlaylists.at(initPlaylist);
	}
	std::string getTestTrack() {
		return initPlaylistTracks.at(initPlaylist);
	}
	int getSpotifyPid() {
		return spotifyPid;
	}
	void setSpotifyPid(int pid) {
		spotifyPid = pid;
	}
	std::string getUri() {
		return uri;
	}
	void setUri(std::string uri) {
		this->uri = uri;
	}
	std::string getUser() {
		return user;
	}
	void setUser(std::string user) {
		this->user = user;
	}
	std::string getPass() {
		return pass;
	}
	void setPass(std::string pass) {
		this->pass = pass;
	}
	std::string getTempLogFile() {
		return tempLogFile;
	}
	std::string getLogFile() {
		return logFile;
	}
	bool getLoginLoggingActive() {
		return loginLoggingActive;
	}

};

#endif /* VARIABLES_H_ */
