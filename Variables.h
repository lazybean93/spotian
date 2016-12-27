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
#include "VirtualDisplay.h"
#include "tools.h"

class Variables {
	std::string commandsDebugFile;
	std::string homedir;
	std::string musicdir;
	std::string datadir;
	std::vector<std::string> initPlaylists;
	std::vector<std::string> initPlaylistTracks;
	int initPlaylist = 1;
	std::string uri;
	std::string user;
	std::string pass;
	std::string tempLogFile;
	std::string logFile;
	bool loginLoggingActive;
	Variables();
	Variables(const Variables&);
	Variables & operator =(const Variables &);
public:
	std::vector<std::string> prefix;
	static Variables& instance();
	~Variables();
	std::string getHomeDir();
	std::string getMusicDir();
	std::string getDataDir();
	std::string getTestPlaylist();
	std::string getTestTrack();
	std::string getUri();
	void setUri(std::string uri);
	std::string getUser();
	void setUser(std::string user);
	std::string getPass();
	void setPass(std::string pass);
	std::string getTempLogFile();
	std::string getLogFile();
	bool getLoginLoggingActive();

};

#endif /* VARIABLES_H_ */
