/*
 * Metadata.h
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#ifndef METADATA_H_
#define METADATA_H_

#include "tools.h"

class Metadata {
	std::string prefix;
	std::string artist;
	std::string album;
	std::string title;
	std::string trackNumber;
	bool ad;
	std::string url;
	int length;
	bool error;
	void setError();
public:
	friend bool operator==(const Metadata &c1, const Metadata &c2) {
		return (c1.artist == c2.artist && c1.album == c2.album
				&& c1.title == c2.title && c1.trackNumber == c2.trackNumber
				&& c1.url == c2.url && c1.error == c2.error);
	};
	friend bool operator!=(const Metadata &c1, const Metadata &c2) {
		return !(c1 == c2);
	};
	std::string getArtist();
	std::string getAlbum();
	std::string getTitle();
	std::string getUrl();
	std::string getTrackNumber();
	int getLength();
	bool isAd();
	bool getError();
	Metadata();
	std::string getFileName(int i);
	void print();
};

#endif /* METADATA_H_ */
