/*
 * Playlist.cpp
 *
 *  Created on: 15.09.2016
 *      Author: oliver
 */

#include "Playlist.h"

std::string Playlist::playlistNameUri(std::string uri) {
	std::string res = "";
	std::vector<std::string> splited = split(uri, ":");
	std::string url = "http://open.spotify.com";
	for (int i = 1; i < (int) splited.size(); i++)
		url += "/" + splited.at(i);

	std::string title = execToString("wget -qO- " + url);
	std::vector<std::string> titleparts = split(title, "og:title\" content=\"");
	titleparts = split(titleparts[1], "\"><meta property=");
	titleparts = split(titleparts[0].substr(18), ", a playlist");
	logline(titleparts[0], true);

	//Check if Playlist is album
	if (titleparts.size() == 1) {
		res = titleparts[0];
		titleparts = split(title,
				"<meta property=\"twitter:audio:artist_name\" content=\"");
		titleparts = split(titleparts[1],
				"\"><meta property=\"twitter:player\" content=");
		res = titleparts[0].substr(51) + " - " + res;
	} else {
		res = titleparts[0] + " - " + date;
	}
	logline(res, true);
	return changeFormat(res);
}
Playlist::Playlist() {
	date = split(execToString("date +%Y.%m.%d"), "\n").at(0);
	prefix = "Playlist:\t";
	name = "";
}
bool Playlist::openPlaylist(std::string uri) {
	bool success = false;
	dbus("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2",
			"org.mpris.MediaPlayer2.Player.OpenUri " + uri, &success);
	if (!success)
		return false;
	return waitLoad(MAXLOADLOADING, 1, true, WAITLOADINGTIMEOUT,
			"Waiting for Playlist Loading");
}
int Playlist::playPlaylist(std::string uri) {
	logline(prefix + "Open URI: " + uri, true);
	if (!openPlaylist(uri))
		return 4022;
	if (!player.pause())
		return 4021;
	execToString("xte 'mousemove 290 230' 'usleep 10000' 'mouseclick 1';");
	do {
		name = playlistNameUri(uri);
	} while (name == "");
	logline(prefix + name, true);
	Timeout timeout = Timeout(60);
	while (!recorder.testPlaying())
		if (!timeout.inTime())
			return 4023;
	currentTrack = 1;
	return 0;
}
void Playlist::readPlaylist() {
	std::vector<Metadata> playlist;

	Metadata current;

	do {
		playlist.push_back(current);
		int errNextSong = nextSong();
		if (errNextSong != 0) {
			logline(prefix + "Error nextSong() = " + to_string(errNextSong),
					true);
			exit(errNextSong);
		}
		current = Metadata();
	} while (current != playlist.at(0) && player.isPlaying());

	currentTrack = 1;

	for (unsigned int i = 0; i < playlist.size(); i++) {
		logline(prefix + playlist.at(i).getFileName(i + 1), true);
	}
	list = playlist;
}
void Playlist::incrementTrack() {
	currentTrack++;
	logline(prefix + "Current trackNumber: " + to_string(currentTrack), true);
}
void Playlist::decrementTrack() {
	currentTrack--;
	logline(prefix + "Current trackNumber: " + to_string(currentTrack), true);
}
int Playlist::nextSong() {

	Timeout timeNext = Timeout(100.0);
	Metadata current = Metadata();
	do {
		Timeout timeChange = Timeout(10.0);
		if (!player.next())
			return 4001;

		while (current == Metadata() && timeChange.inTime())
			;
		if (current != Metadata()) {
			incrementTrack();
			return 0;
		}
	} while (timeNext.inTime());
	return 4002;
}
int Playlist::seek(int i) {
	//move to Track number i and pause at beginning

	logline(prefix + "seeking to track number " + to_string(i), true);

	if (i > 1) { //Seeking to first Track means "do nothing!"
		int err4002count = 0;

		Metadata current;
		while (current != list.at(i - 1)) {
			int errNextSong = nextSong();
			if (errNextSong == 4001) {
				logline(prefix + "Error nextSong() = " + to_string(errNextSong),
						true);
				return 4011;
			}
			if (errNextSong == 4002) {
				err4002count++;
				if (err4002count > 10) {
					logline(
							prefix
									+ "Error nextSong() failed too often with error code 4002",
							true);
					return 4012;
				}
			}
			if (errNextSong == 0) {
				err4002count = 0;
				current = Metadata();
			}
		}

		Timeout timeout = Timeout(60);
		while (!recorder.testPlaying())
			if (!timeout.inTime())
				return 4014;

		sleep(4);
		if (!player.pause() || !player.previous())
			return 4013;
	}
	return 0;
}
std::string Playlist::getName() {
	return name;
}
Metadata Playlist::at(int i) {
	return list.at(i);
}
int Playlist::size() {
	return list.size();
}
void Playlist::createFolder() {
	folder = "tmp/" + getName() + "/";

	//Create PlaylistFolder
	mkdir (folder);
}
std::string Playlist::getFolder() {
	return folder;
}
bool Playlist::isFinished() {
	std::string folder = "tmp/" + getName() + "/";
	std::string songcountSTR = execToString(
			"ls \"" + folder + "\" 2>/dev/null | wc -l");
	int songcount = atoi(songcountSTR.c_str());
	logline(to_string(songcount) + ' ' + to_string(size()), true);
	return (at(0) == Metadata() && songcount == size());
}

