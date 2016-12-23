/*
 * Metadata.cpp
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#include "Metadata.h"

void Metadata::setError() {
	error = true;
	artist = "";
	album = "";
	title = "";
	url = "";
	trackNumber = "";
	length = 0;
	ad = false;
	logline("Got wrong Metadata", true);
	return;
}
std::string Metadata::getArtist() {
	return artist;
}
std::string Metadata::getAlbum() {
	return album;
}
std::string Metadata::getTitle() {
	return title;
}
std::string Metadata::getUrl() {
	return url;
}
std::string Metadata::getUrlMP3() {
	return url+".mp3";
}
std::string Metadata::getTrackNumber() {
	return trackNumber;
}
int Metadata::getLength() {
	return length;
}
bool Metadata::isAd() {
	return ad;
}
bool Metadata::getError() {
	return error;
}
Metadata::Metadata() {
	prefix = "Metadata:\t";
	std::vector<std::string> meta;
	bool success;
	std::string metadataString = dbus("org.mpris.MediaPlayer2.spotify",
			"/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player.Metadata",
			&success);
	if (!success) {
		setError();
		return;
	}
	meta = split(metadataString, "\n");
	if (meta.size() < 11) {
		setError();
		return;
	}
	int cMeta = meta.size();
	for (int i = 0; i < cMeta; i++) {
		std::vector<std::string> metaline = split(meta.at(i), ":");
		meta.at(i) = "";
		int cMetaline = metaline.size();
		for (int j = 2; j < cMetaline - 1; j++) {
			meta.at(i) += metaline.at(j);
			meta.at(i) += ":";
		}
		meta.at(i) += metaline.at(cMetaline - 1);
		meta.at(i).erase(0, 1);
		meta.at(i) = changeFormat(meta.at(i));
	}
	error = false;
	artist = meta.at(4);
	album = meta.at(3);
	title = meta.at(8);
	url = split(meta.at(2), ":").at(2);
	trackNumber = meta.at(9);
	length = atol(meta.at(1).c_str()) / 1000000;
	ad = (meta.at(2).find("spotify:ad:") < 3
			|| split(meta.at(2), ":").at(2).size() == 32);
	return;
}
std::string Metadata::getFileName(int i) {

	//4+5*3 = 24 -> 89 / 3 frei
	int maxlen = 89/3;
	std::string number = numToString(i);
	while (number.length() < 4)
		number = "0" + number;
	std::string fileName = number; 			//4
	fileName += " - "; 						//3
	fileName += toUTF8(artist.substr(0, maxlen)); 	//
	fileName += " - "; 						//3
	fileName += toUTF8(album.substr(0, maxlen));	//
	fileName += " - ";						//3
	fileName += trackNumber;				//3
	fileName += " - ";						//3
	fileName += toUTF8(title.substr(0, maxlen));	//
	fileName += ".mp3";
	return fileName;
}
void Metadata::print() {
	logline(prefix + "Artist:\t" + artist, true);
	logline(prefix + "Album:\t" + album, true);
	logline(prefix + "Title:\t" + title, true);
	logline(prefix + "TrackNumber:\t" + trackNumber, true);
	logline(prefix + "Url:\t" + url, true);
	logline(prefix + "Length:\t" + numToString(length), true);
}

