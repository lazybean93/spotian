/*
 * Playlist.h
 *
 *  Created on: 15.09.2016
 *      Author: oliver
 */

#ifndef PLAYLIST_H_
#define PLAYLIST_H_

#include <vector>
#include "Metadata.h"
#include "Player.h"
#include "Recorder.h"
#include "tools.h"

class Playlist {
	std::vector<Metadata> list;
	std::string prefix;
	std::string name;
	std::string date;
	std::string folder;
	int currentTrack;
	Playlist();
	Playlist(const Playlist&);
	Playlist & operator =(const Playlist &);
public:
	static Playlist& instance() {
		static Playlist _instance;
		return _instance;
	}
	~Playlist() {
	}
	std::string playlistNameUri(std::string uri,bool addDate = true);
	bool openPlaylist(std::string uri);
	int playPlaylist(std::string uri);
	void readPlaylist();
	void incrementTrack();
	void decrementTrack();
	int nextSong();
	int seek(int i);
	std::string getName();
	Metadata at(int i);
	int size();
	void createFolder();
	std::string getFolder();
	bool isFinished();
};

#endif /* PLAYLIST_H_ */
