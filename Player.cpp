/*
 * Player.cpp
 *
 *  Created on: 14.09.2016
 *      Author: oliver
 */

#include "Player.h"

bool Player::controlPlayer(std::string action) {
	std::string destination = "org.mpris.MediaPlayer2.spotify";
	std::string path = "/org/mpris/MediaPlayer2";
	std::string interface = "org.mpris.MediaPlayer2.Player";

	DBusConnection* bus = NULL; //der Bus
	DBusMessage* msg = NULL; //meine Nachricht
	DBusError error; //Datenstruktur für Fehler

	dbus_error_init(&error); //Fehler Datenstruktur init

	bus = dbus_bus_get(DBUS_BUS_SESSION, &error); //Verbinde mit DBus Sitzung
	if (dbus_error_is_set(&error))
		return false;

	//Prüfe ob Spotify läuft bzw mit Bus verbunden ist
	if (!dbus_bus_name_has_owner(bus, destination.c_str(), &error)) {
		fprintf(stderr, "Name has no owner on the bus!\n");
		return false;
	}
	if (dbus_error_is_set(&error))
		return false;

	//Erstelle Nachricht
	msg = dbus_message_new_method_call(destination.c_str(), // destination
			path.c_str(), // obj. path
			interface.c_str(), // interface
			action.c_str()); // method str
	if (msg == NULL) {
		fprintf(stderr, "Ran out of memory when creating a message\n");
		return false;
	}

	//Ich möchte keine Antwort!
	dbus_message_set_no_reply(msg, TRUE);

	//Packe Nachricht in Warteschlange
	if (!dbus_connection_send(bus, msg, NULL)) {
		fprintf(stderr, "Ran out of memory while queueing message\n");
		return false;
	}

	//Warte, bis Warteschlange leer
	dbus_connection_flush(bus);

	//Gebe Speicher der Nachricht Frei
	dbus_message_unref(msg);
	msg = NULL;

	//Gebe Speicher der Busverbindung Frei
	dbus_connection_unref(bus);
	bus = NULL;

	return true;

}
Player::Player() {
	playing = false;
	prefix = "Player:\t";
}
bool Player::getPlaying() {
	return playing;
}
bool Player::isPlaying() {
	return (dbus("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2",
			"org.mpris.MediaPlayer2.Player.PlaybackStatus").find("Play")
			!= std::string::npos);
}
void Player::setPlaying(bool set) {
	playing = set;
}
bool Player::pause() {
	logline(prefix + "pause!", true);
	bool res = controlPlayer("Pause");
	if (!res) //sending Pause failed
		return false;

	bool wait = waitLoad(MAXLOADPAUSED, 0.2, true, WAITLOADTIMEOUTPAUSED, 0,
			prefix + "after Pause");
	if (!wait) {
		//load too high, second attempt
		bool res = controlPlayer("Pause");
		if (!res) //sending Pause failed
			return false;

		wait = waitLoad(MAXLOADPAUSED, 0.2, true, WAITLOADTIMEOUTPAUSECHECK, 0,
				prefix + "after Pause");
		if (!wait)
			return false; //load too high!
	}

	//paused correctly
	playing = false;
	return true;

}
bool Player::play() {
	logline(prefix + "play!", true);
	bool res = waitLoad(MAXLOADPAUSED, 0.2, true, WAITLOADTIMEOUTPAUSED, 0,
			prefix + "before Play");
	if (!res)
		return false;
	res = controlPlayer("PlayPause");
	if (!res)
		return false;
	//send correctly
	playing = true;
	return true;
}
bool Player::previous() {
	bool res = controlPlayer("Previous");
	if (res)
		logline(prefix + "previous", true);
	//pause mitten im lied -> pause, anfang vom gleichen lied
	//pause, anfang vom lied -> play, lied davor
	//play -> play
	return res;
}
bool Player::next() {
	bool res = controlPlayer("Next");
	//spielt immer ab, außer lied ist das letzte in der playlist
	return res;
}
bool Player::pauseAtStart() {
	if (!isPlaying()) {
		if (!play())
			return false;
	}
	//Playing Song
	sleep(4);
	if (!pause() || !previous()) {
		return false;
	}
	return true;
}

