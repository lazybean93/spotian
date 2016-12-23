/*
 * TODO:
 *  - Playlist besser einbinden
 *  	- Next soll TrackId kennen -> Play Status lässt sich erkennen
 *  - Check ob lieder vorhanden allein anhand von Playlist
 *  - Dateiname darf keine zeichen abseits von utf-8 enthalten
 */

#include "variables.h"
#include "tools.h"
#include "timeout.h"
#include "Metadata.h"
#include "Recorder.h"
#include "Player.h"
#include "Playlist.h"
#include <fstream>

void killSpotify() {
	bool success = true;
	if (variables::instance().getSpotifyPid() == -1)
		variables::instance().setSpotifyPid(
				getProcIdByName("/usr/share/spotify/spotify"));
	while (success && variables::instance().getSpotifyPid() != -1) {
		kill(variables::instance().getSpotifyPid(), SIGTERM);
		readFile(
				"/proc/" + numToString(variables::instance().getSpotifyPid())
						+ "/stat", &success);
		usleep(0.1 * 1000000);
	}
	variables::instance().setSpotifyPid(-1);
	deleteFolder(variables::instance().getHomeDir() + "/.cache/spotify");
}
void startSpotify(std::string spotLogFile) {
	logline("startSpotify", true);
	killSpotify();
	variables::instance().setMainPage(true);
	execToString(
			"rm .config/spotify/Users/lazybean93-user/*.bnk >/dev/null 2>&1 &");
	variables::instance().setSpotifyPid(
			atoi(
					execToString(
							"spotify 1>/dev/null 2>" + spotLogFile
									+ " & echo $!").c_str()));

}
void resetCache() {
	deleteFolder(variables::instance().getHomeDir() + "/.cache/spotify");
	deleteFolder(variables::instance().getHomeDir() + "/.config/spotify");
}
int checkError(std::string in, std::string out) {
	//Moves content of logfile to out
	//Returns errorcode
	bool success;
	std::string content = readFile(in, &success) + "\n";
	if (!success) {
		return 0;
	}
	//Only Work if File has content, otherwise no error!
	if (content.length() > 1) {
		//Move content to our
		int lines = strfind(content, "\n");
		std::string head = execToString(
				"head -n " + numToString(lines) + " " + in);
		std::string tail = execToString(
				"tail -n +" + numToString(lines) + " " + in);
		writeToFile(head, out, true);
		writeToFile(tail, in, false);
		//Do the real check
		if (std::string::npos
				!= content.find("Locale could not be found for en-US"))
			return 1; //Kein Plan!
		if (std::string::npos != content.find("Connection error:  4"))
			return 2;
		if (std::string::npos != content.find("Connection error:  117"))
			return 2; //Kein Internet! Auch im Betrieb interessant!
		if (std::string::npos != content.find("Connection error:  410"))
			return 3; //Benutzername oder Passwort Falsch!
		if (std::string::npos != content.find("file not found in archive:"))
			return 4; //failed loading skin!
		//if (std::string::npos
		// != content.find(
		// "Uncaught unknown:  (bridge message: 'user_metadata', args: [\"spotify:user:@\"])"))
		// return 5; //failed loading skin!
		if (std::string::npos != content.find("Failed to launch child process"))
			return 5; //???
		if (std::string::npos != content.find("Cannot communicate with zygote"))
			return 6; //???
		if (std::string::npos
				!= content.find("Deadlock detected (Thread: gui)"))
			return 7; //???
		if (std::string::npos != content.find("Fatal deadlock detected"))
			return 8; //
		if (std::string::npos
				!= content.find(
						"cannot open variables::instance().getDisplay()"))
			return 9; //
	}
	return 0;
}
bool checkLoaded(std::string log) {
	std::string line;
	std::ifstream myfile(log);
	int process = 0;
	bool created = false;
	if (myfile.is_open()) {
		while (std::getline(myfile, line)) {
			if (line.find("playlist_be_pl4_context.cpp:71") < line.size()) {
				process++;
				created = true;
				//std::cout << line << '\n';
			}
			if (line.find("playlist_be_pl4_context.cpp:202") < line.size()) {
				process--;
				//std::cout << line << '\n';
			}

		}
		myfile.close();
		return (created && process == 0);
	}

	//std::cout << "Unable to open file";
	logline("Unable to open file", true);
	return false;
}
int login(std::string user, std::string pass, std::string spotLogFile,
		std::string checkedLog, bool logActive) {

	int windowWidth = 904;
	int windowHeight = 668;
	resetCache();
	startSpotify(spotLogFile);
	while (execToString(
			"wmctrl -lG 2>/dev/null | grep " + numToString(windowWidth)
					+ " | grep " + numToString(windowHeight)) == "") {
		int err = checkError(spotLogFile, checkedLog);
		if (err > 0) {
			return err;
		}
		if (!waitLoad(MAXLOADLOADING, 0.5, logActive, WAITLOADINGTIMEOUT,
				"Waiting for LoginWindow"))
			return 1002;
	}
	//Move Window
	logline("Move Window", logActive);
	//while (execToString("wmctrl -lG | grep \"0   320  500  Spotian spotify\"") == "") {
	while (execToString(
			"wmctrl -lG  2>/dev/null | grep \"16   " + numToString(windowWidth)
					+ "  " + numToString(windowHeight) + "\" | grep Spotify")
			== "") {
		logline("Check for Error", logActive);
		int err = checkError(spotLogFile, checkedLog);
		if (err > 0) {
			return err;
		}
		execToString(
				"wmctrl -r spotify -e 0,0,0," + numToString(windowWidth) + ","
						+ numToString(windowHeight));
		if (!waitLoad(MAXLOADLOADING, 0.5, logActive, WAITLOADINGTIMEOUT))
			return 1003;
	}
	//Enter Userdata, and Login
	logline("Enter Userdata, and Login", logActive);
	float time = 0.5;
	execToString("xte 'mousemove 470 290'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1004;
	execToString("xte 'mouseclick 1'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1005;
	execToString("xte 'str " + user + "'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1006;
	execToString("xte 'mousemove 470 340'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1007;
	execToString("xte 'mouseclick 1'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1008;
	execToString("xte 'str " + pass + "'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1009;
	execToString("xte 'mousemove 470 430'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1010;
	execToString("xte 'mouseclick 1'");

	//Wait till initialisation is done
	logline("Check if Playlists are Loaded", logActive);
	waitLoad(MAXLOADLOADING, 1.0, true, WAITLOADTIMEOUTPAUSED, "Loaded Window");
	int j = 0;
	while (!checkLoaded(spotLogFile) && j < 10) {
		sleep(1);
		j++;
	}
	execToString("wmctrl -r Spotify -b toggle,maximized_vert,maximized_horz");

	int err = checkError(spotLogFile, checkedLog);
	if (err > 0) {
		return err;
	}

	if (!Playlist::instance().openPlaylist(
			variables::instance().getTestPlaylist()))
		return 1011;
	if (!Player::instance().pause())
		return 1012;

	float cpuload;
	float maxload = 1.0;
	float avgLoadTime = 0.5;

	do {
		int err = checkError(spotLogFile, checkedLog);
		if (err > 0) {
			logline("error! " + numToString(err), logActive);
			return err;
		}
		cpuload = avgCPULoad(avgLoadTime);
		logline("CPU-Load: " + numToString(cpuload), logActive);
	} while (cpuload > maxload);
//	//No local library
//	execToString("xte 'keydown Control_R' 'key p' 'keyup Control_R'");
//	sleep(1);
//	execToString("xte 'mousemove 237 624' 'mouseclick 1'");
//	sleep(1);
//	execToString("xte 'mousemove 34 103' 'mouseclick 1'");
//	sleep(2);

	//Check data
	if (atoi(
			execToString(
					"cat " + variables::instance().getHomeDir()
							+ "/.config/spotify/prefs | grep autologin | wc -l").c_str())
			< 3) {
		logline(
				execToString(
						"cat " + variables::instance().getHomeDir()
								+ "/.config/spotify/prefs | grep autologin | wc -l"),
				logActive);
		return 1001;
	}
	//Add audio gap between songs
	logline("Add audio gap between songs", logActive);
	writeToFile("audio.crossfade_v2=false",
			".config/spotify/lazybean93-user/prefs", false);
	logline("Login finished", true);
	return 0;

}
void doLogin() {
	variables::instance().prefix.push_back("Login");
	variables& var = variables::instance();
	int err = 0;
	do {
		err = login(var.getUser(), var.getPass(), var.getTempLogFile(),
				var.getLogFile(), var.getLoginLoggingActive());
		if (err > 0)
			logline("Login Failed, errcode: " + numToString(err), true);
	} while (err > 0);

	variables::instance().prefix.pop_back();
}
int init(std::string uri, std::string spotLogFile) {
	//Only needed to press play and record if 0 will be returned

	Player& player = Player::instance();
	Playlist& playlist = Playlist::instance();

	//Doing Soundcheck

	variables::instance().prefix.push_back("Soundcheck");

	bool err;
	do {
		err = false;
		logline("Soundcheck Failed! Try again", true);
		startSpotify(spotLogFile);
		logline("Spotify Started", true);
		Timeout startTimeout(60);
		while (!checkLoaded(spotLogFile) && startTimeout.inTime())
			sleep(0.1);
		if (!startTimeout.inTime())
			return 2014;
		execToString(
				"wmctrl -r Spotify -b toggle,maximized_vert,maximized_horz");
		if (playlist.playPlaylist(variables::instance().getTestPlaylist()) != 0)
			err = true;
		if (!player.pause())
			return 2012;

		if (!waitLoad(MAXLOADLOADING, 1.5, true, WAITLOADINGTIMEOUT))
			return 2010;

		if (!player.play())
			return 2013;
		sleep(10);
	} while (err || !Recorder::instance().testPlaying());

	variables::instance().prefix.pop_back();

	//Set Playlist
	variables::instance().prefix.push_back("Set Playlist");

	Metadata m1;

	if (playlist.playPlaylist(uri) != 0) {
		return 2001;
	}

	//Go to beginning of first Track
	if (!player.pauseAtStart()) {
		return 2002;
	}

	//Read Playlist
	playlist.readPlaylist();

	//Create Folder
	playlist.createFolder();

	variables::instance().prefix.pop_back();

	return 0;
}
void doInit() {
	variables::instance().prefix.push_back("Init\t");

	while (init(variables::instance().getUri(),
			variables::instance().getTempLogFile()) > 0)
		variables::instance().prefix.pop_back();

	variables::instance().prefix.pop_back();

}
int checkCorrectSong(Metadata m1, int num) {
	Playlist& playlist = Playlist::instance();
	if (m1 != playlist.at(num - 1)) {
		logline("Metadata does not match Playlist", true);
		logline("Metadata:", true);
		m1.print();
		logline("Playlist:", true);
		playlist.at(num - 1).print();
		logline("Check if song should be next song", true);

		if (m1 == playlist.at(num - 2)) {
			if (playlist.nextSong() != 0)
				return 4010;
			playlist.decrementTrack();
			m1 = Metadata();
			if (m1 != playlist.at(num - 1))
				return 4011;
			if (!Player::instance().pauseAtStart())
				return 4012;
		}
		m1 = Metadata();
		if (m1 != playlist.at(num - 1))
			return 5001;
	}
	return 0;
}
int moveToSong(int num) {
	Playlist& playlist = Playlist::instance();

	for (int i = 0; i < playlist.size(); i++) {
		if (Metadata() == playlist.at(i)) {
			logline("Spotify is at Song " + numToString(i), true);
			logline("Spotian is at Song " + numToString(num - 1), true);
			if (num - i > 1)
				playlist.seek(num);
			break;
		}
	}

	return 0;
}
std::string findSong(Metadata m1) {
	std::vector<std::string> files = split(
			execToString(
					"find * -type f | fgrep \"" + m1.getFileName(0).substr(7)
							+ "\""), "\n");
	for (int i = 0; i < (int) files.size(); i++) {
		std::vector<std::string> tmp_filename_vector = split(files.at(i), "/");
		std::string tmp_filename = "";
		for (int j = 0; j < (int) tmp_filename_vector.size() - 1; j++) {
			tmp_filename += tmp_filename_vector.at(j) + "/";
		}
		tmp_filename +=
				tmp_filename_vector.at(tmp_filename_vector.size() - 1).substr(0,
						7);

		std::string cmd = "id3tool \"" + tmp_filename + "\"* | grep "
				+ m1.getUrl();
		std::string result = execToString(cmd);
		//logline(result, true);
		if (result != "")
			return tmp_filename;
	}
	return "";
}
int copySong(Metadata m, int num) {
	Playlist& playlist = Playlist::instance();
	std::string copy = findSong(m);

	if (copy != "") {
		logline("Copy from old Recordings", true);
		std::string cmd = "cp \"" + string_replace(copy, "\"", "\\\"")
				+ "\"* \"" + variables::instance().getDataDir() + "/"
				+ m.getUrlMP3() + "\"";
		execToString(cmd);
	} else
		return 1;
	return 0;
}
void printCurrentSong(int track, int size, Metadata song) {
	std::string num_str = numToString(track);
	while (num_str.size() < numToString(size).size())
		num_str = "0" + num_str;

	logline(num_str + "/" + numToString(size) + " " + song.getFileName(track),
			true);
}
void setSymlink(Metadata m, int num) {
	execToString(
			"cd " + variables::instance().getDataDir() + "; ln -sf ../data/"
					+ m.getUrlMP3() + " \"../" + Playlist::instance().getName()
					+ "/" + m.getFileName(num) + "\"");
}
int recordSong(int num) {
//State: Spotify is Started, Playlist is Loaded, track is not Playing

	Player& player = Player::instance();
	Recorder& recorder = Recorder::instance();
	Playlist& playlist = Playlist::instance();

//Echo current Song
	printCurrentSong(num, playlist.size(), playlist.at(num - 1));
	if (checkCorrectSong(Metadata(),num)>0)
		return 5002;

//Search for Song
	if (!is_file_exist(
			variables::instance().getDataDir() + "/"
					+ playlist.at(num - 1).getUrlMP3())) {
		int copy = copySong(playlist.at(num - 1), num);
		if (copy == 1) {

			//Compare metadata to Playlist
			Metadata m1;

			if (!player.pauseAtStart())
				return 5010;
			//Start Recording

			int errRecording = recorder.recordSong(m1, num);
			if (errRecording > 999)
				return errRecording;
			bool ad = false;
			if (errRecording == 1)
				ad = true;

			//Track has been recorded. Invisible ad has been played.
			//Next Track, or visible ad is playing

			Metadata m2;
			if (m2.isAd()) {
				logline("Got more ads", true);
				Timeout adTimeout(ADTIMEOUT);
				while (m2.isAd()) {

					if (!adTimeout.inTime())
						return 5004;
					sleep(2);
					m2 = Metadata();
				}
				logline("Got beyond ad", true);
			}

			//Next Track is playing, check for metadata
			if (m2.getError())
				return 5005;

			//Next Track is not playing after Advertisement
			if (ad && !player.isPlaying()) {
				logline("Had advertisement, and now... something went wrong",
						true);
				if (m2 != playlist.at(0)) {
					//What the hell is happening here????
					sleep(5);
					if (!player.isPlaying()) {
						player.play();
						sleep(5);
						if (player.isPlaying()) {
							m2 = Metadata();
							m2.print();
							int correctSongErr = checkCorrectSong(m2, num + 1);
							if (correctSongErr > 0)
								return correctSongErr;
						} else
							return 5006;
					}
				}
			}

			//Check Song, eventually short it

			if (!recorder.checkOverrun())
				return 5007;		//check for overrun
			logline("Check Song", true);
			recorder.resetSongErrors(2);

			bool check = recorder.checkSong(
					variables::instance().getDataDir() + "/" + m1.getUrlMP3(),
					true);

			if (!check) {
				logline("Check Failed, record again!", true);
				return 5008;
			}

			logline("Finished Recording", true);

			if (player.isPlaying()) {
				playlist.incrementTrack();
			}
		}
	} else {
		int err = playlist.nextSong();
		if (err>0) return err;
	}

	setSymlink(playlist.at(num - 1), num);

	return 0;
}
void doRecord() {
	variables::instance().prefix.push_back("Record");
	Playlist& playlist = Playlist::instance();

//Track is from init Playlist -> Playlist is not available
	if (Metadata().getUrl() != variables::instance().getTestTrack()) {
		int i = 1;

		do {
			int err = recordSong(i);
			if (err > 0) {
				deleteFile(
						variables::instance().getDataDir() + "/"
								+ playlist.at(i).getUrlMP3());
				logline("Recording Failed, errcode: " + numToString(err), true);
				doInit();

				i = 1;
			} else
				i++;
		} while (!playlist.isFinished());
	} else {
		logline("Playlist is not available", true);
	}
	variables::instance().prefix.pop_back();
}
void doExit() {
	killSpotify();
	std::string killdisplay = execToString(
			"tightvncserver -kill " + variables::instance().getDisplay());
}
bool doLoad(int argc, char* argv[]) {
	variables& var = variables::instance();
	var.prefix.push_back("Load\t");

	deleteFile("commands.txt");

	logline(
			"Compiled on " + std::string(__DATE__) + " at "
					+ std::string(__TIME__), true);
//Some Variables

	var.setUser(argv[1]);
	var.setPass(argv[2]);
	var.setUri(argv[3]);

	var.prefix.pop_back();
	return true;

}
int main(int argc, char* argv[]) {

	Timeout t0(0);

	if (!doLoad(argc, argv))
		return false;

	doLogin();

	std::string timelogin = t0.getFormatTime();
	Timeout t1(0);

	doInit();

	std::string timeinit = t1.getFormatTime();
	Timeout t2(0);

	doRecord();

	doExit();

	std::string timerecord = t2.getFormatTime();
	std::string timetotal = t0.getFormatTime();

	logline(
			"Elapsed time: " + timetotal + " | " + timelogin + " | " + timeinit
					+ " | " + timerecord, true);
}
