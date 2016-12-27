/*
 * Variables.cpp
 *
 *  Created on: 26.12.2016
 *      Author: oliver
 */

#include "Variables.h"

Variables::Variables() {
	homedir = std::string(getenv("HOME"));
	musicdir = homedir + "/tmp";
	datadir = musicdir + "/data";

	mkdir(musicdir);
	mkdir(datadir);

	initPlaylists.push_back("spotify:album:2ppmPxlKzpaJy23sbe3vq8");
	initPlaylistTracks.push_back("spotify:track:3Z58ppU1Q7nwAfwCWCTRjv");

	initPlaylists.push_back("spotify:album:40lxWZ19hZgH0s71KGXOyo");
	initPlaylistTracks.push_back("spotify:track:4YuGhuEypxkqhd3QdewSuk");

	loginLoggingActive = true;
}
Variables& Variables::instance() {
	static Variables _instance;
	return _instance;
}
Variables::~Variables() {
};
std::string Variables::getHomeDir() {
	return homedir;
}
std::string Variables::getMusicDir() {
	return musicdir;
}
std::string Variables::getDataDir() {
	return datadir;
}
std::string Variables::getTestPlaylist() {
	initPlaylist = (initPlaylist + 1) % initPlaylists.size();
	return initPlaylists.at(initPlaylist);
}
std::string Variables::getTestTrack() {
	return initPlaylistTracks.at(initPlaylist);
}
std::string Variables::getUri() {
	return uri;
}
void Variables::setUri(std::string uri) {
	this->uri = uri;
}
std::string Variables::getUser() {
	return user;
}
void Variables::setUser(std::string user) {
	this->user = user;
}
std::string Variables::getPass() {
	return pass;
}
void Variables::setPass(std::string pass) {
	this->pass = pass;
}
std::string Variables::getTempLogFile() {
	return getHomeDir() + "/spotifytemplog_" + VirtualDisplay::instance().getVirtualDisplay() + ".txt";;
}
std::string Variables::getLogFile() {
	return getHomeDir() + "/spotifylog_" + VirtualDisplay::instance().getVirtualDisplay() + ".txt";;
}
bool Variables::getLoginLoggingActive() {
	return loginLoggingActive;
}
