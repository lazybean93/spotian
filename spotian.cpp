/*
 * TODO:
 *  - Playlist besser einbinden
 *  	- Next soll TrackId kennen -> Play Status lässt sich erkennen
 *  - Check ob lieder vorhanden allein anhand von Playlist
 */

#include "variables.h"
#include "tools.h"
#include "timeout.h"
#include "Metadata.h"
#include "Recorder.h"
#include "Player.h"
#include "Playlist.h"
#include <fstream>

bool paramCountWrong(int argc) {
	if (argc != 4) {
		std::cout << "Parameter fehlerhaft!" << std::endl;
		return true;
	}
	return false;
}
void killSpotify() {
	bool success = true;
	if (variables::instance().getSpotifyPid() == -1)
		variables::instance().setSpotifyPid(
				getProcIdByName("/usr/share/spotify/spotify"));
	while (success && variables::instance().getSpotifyPid() != -1) {
		kill(variables::instance().getSpotifyPid(), SIGTERM);
		readFile(
				"/proc/" + to_string(variables::instance().getSpotifyPid())
						+ "/stat", &success);
		usleep(0.1 * 1000000);
	}
	variables::instance().setSpotifyPid(-1);
	deleteFolder(variables::instance().getHomedir() + "/.cache/spotify");
}
void startSpotify(std::string spotLogFile) {
	killSpotify();
	execToString("rm .config/spotify/Users/lazybean93-user/*.bnk");
	variables::instance().setSpotifyPid(
			atoi(
					execToString(
							"spotify 1>/dev/null 2>" + spotLogFile
									+ " & echo $!").c_str()));

}
void resetCache() {
	deleteFolder(variables::instance().getHomedir() + "/.cache/spotify");
	deleteFolder(variables::instance().getHomedir() + "/.config/spotify");
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
				"head -n " + to_string(lines) + " " + in);
		std::string tail = execToString(
				"tail -n +" + to_string(lines) + " " + in);
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
	logline("startSpotify", logActive);
	resetCache();
	startSpotify(spotLogFile);
	while (execToString(
			"wmctrl -lG | grep " + to_string(windowWidth) + " | grep "
					+ to_string(windowHeight)) == "") {
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
			"wmctrl -lG | grep \"16   " + to_string(windowWidth) + "  "
					+ to_string(windowHeight) + "\" | grep Spotify") == "") {
		logline("Check for Error", logActive);
		int err = checkError(spotLogFile, checkedLog);
		if (err > 0) {
			return err;
		}
		execToString(
				"wmctrl -r spotify -e 0,0,0," + to_string(windowWidth) + ","
						+ to_string(windowHeight));
		if (!waitLoad(MAXLOADLOADING, 0.5, logActive, WAITLOADINGTIMEOUT))
			return 1003;
	}
	//Enter Userdata, and Login
	logline("Enter Userdata, and Login", logActive);
	float time = 0.2;
	execToString("xte 'mousemove 470 270'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1004;
	execToString("xte 'mouseclick 1'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1005;
	execToString("xte 'str " + user + "'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1006;
	execToString("xte 'mousemove 470 320'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1007;
	execToString("xte 'mouseclick 1'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1008;
	execToString("xte 'str " + pass + "'");
	if (!waitLoad(MAXLOADLOADING, time, false, WAITLOADINGTIMEOUT))
		return 1009;
	execToString("xte 'mousemove 470 420'");
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
			"spotify:album:40lxWZ19hZgH0s71KGXOyo"))
		return 1011;

	float cpuload;
	float maxload = 1.0;
	float avgLoadTime = 0.5;

	do {
		int err = checkError(spotLogFile, checkedLog);
		if (err > 0) {
			logline("error! " + to_string(err), logActive);
			return err;
		}
		cpuload = avgCPULoad(avgLoadTime);
		logline("CPU-Load: " + to_string(cpuload), logActive);
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
					"cat " + variables::instance().getHomedir()
							+ "/.config/spotify/prefs | grep autologin | wc -l").c_str())
			< 3) {
		logline(
				execToString(
						"cat " + variables::instance().getHomedir()
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
int init(std::string uri, std::string spotLogFile) {
	//Only needed to press play and record if 0 will be returned

	Player& player = Player::instance();
	Playlist& playlist = Playlist::instance();
	//Doing Soundcheck
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
		if (playlist.playPlaylist("spotify:album:40lxWZ19hZgH0s71KGXOyo") != 0)
			err = true;
		if (!player.pause())
			return 2012;

		if (!waitLoad(MAXLOADLOADING, 1.5, true, WAITLOADINGTIMEOUT))
			return 2010;

		if (!player.play())
			return 2013;
		sleep(10);
	} while (err || !Recorder::instance().testPlaying());

	//Set Playlist
	if (playlist.playPlaylist(uri) != 0) {
		return 2001;
	}

	sleep(5);
	//Go to beginning of first Track
	if (!player.pause() || !player.previous()) {
		return 2002;
	}
	logline("Finish", true);
	playlist.readPlaylist();
	playlist.createFolder();
	return 0;
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
int copySong(Metadata m1, int num) {
	Playlist& playlist = Playlist::instance();
	std::string copy = findSong(m1);

	if (copy != "") {
		logline("Copy from old Recordings", true);
		std::string cmd = "cp \"" + string_replace(copy, "\"", "\\\"")
				+ "\"* \"" + playlist.getFolder() + m1.getFileName(num) + "\"";
		execToString(cmd);
		int errNextSong = playlist.nextSong();
		if (errNextSong != 0) {
			return 5009;
		}
	} else
		return 1;
	return 0;
}
int recordSong(int num) {
//State: Spotify is Started, Playlist is Loaded, track is not Playing

	Player& player = Player::instance();
	Recorder& recorder = Recorder::instance();
	Playlist& playlist = Playlist::instance();

//Compare metadata to Playlist
	Metadata m1;
	int correctSongErr = checkCorrectSong(m1, num);
	if (correctSongErr > 0)
		return correctSongErr;

	logline(m1.getFileName(num), true);

//Remove old version of Song
	std::string full_number = to_string(num);
	while (full_number.length() < 4)
		full_number = "0" + full_number;
	deleteOld(playlist.getFolder(), full_number);

//Search for Song
	int copy = copySong(m1, num);
	if (copy > 999)
		return copy;
	if (copy == 1) {
		if (!player.pauseAtStart())
				return 5010;
		//Start Recording

		int errRecording = recorder.recordSong(m1, playlist.getFolder(), num);
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
			logline("Had advertisement, and now... something went wrong", true);
			if (m2 != playlist.at(0)) {
				//What the hell is happening here????
				sleep(5);
				if (!player.isPlaying()) {
					player.play();
					sleep(5);
					if (player.isPlaying()) {
						m2 = Metadata();
						m2.print();
						int correctSongErr = checkCorrectSong(m2, num+1);
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

		execToString(
				"mv \"" + variables::instance().getHomedir() + "/"
						+ playlist.getFolder() + m1.getFileName(num) + "\" \""
						+ variables::instance().getHomedir()
						+ "/tmp_check.mp3\"");
		bool check = recorder.checkSong(
				variables::instance().getHomedir() + "/tmp_check.mp3", true);
		execToString(
				"mv \"" + variables::instance().getHomedir()
						+ "/tmp_check.mp3\" \""
						+ variables::instance().getHomedir() + "/"
						+ playlist.getFolder() + m1.getFileName(num) + "\"");

		if (!check) {
			deleteOld(playlist.getFolder(), full_number);
			logline("Check Failed, record again!", true);
			return 5008;
		}

		logline("Finished Recording", true);

		if (player.isPlaying()) {
			playlist.incrementTrack();
		}
	}

	return 0;
}
int main(int argc, char* argv[]) {

	deleteFile("commands.txt");

	if (paramCountWrong(argc))
		return 0;	//Check Parameters

	std::cout << "Compiled on " __DATE__ << " at " << __TIME__ << std::endl
			<< std::endl;
//Some Variables

	std::string User = argv[1];
	std::string Pass = argv[2];
	std::string templog = variables::instance().getHomedir()
			+ "/spotifytemplog_" + variables::instance().getDisplay() + ".txt";
	std::string log = variables::instance().getHomedir() + "/spotifylog_"
			+ variables::instance().getDisplay() + ".txt";
	std::string uri = argv[3];

	bool loginloggingactive = true;

//Login
	int err = 0;
	do {
		err = login(User, Pass, templog, log,
				loginloggingactive);
		if (err > 0)
			logline("Login Failed, errcode: " + to_string(err), true);
	} while (err > 0);

//Init

	while (init(uri, templog) > 0)
		;

	Playlist& playlist = Playlist::instance();

	//Track is from init Playlist -> Playlist is not available
	if (Metadata().getUrl() != "spotify:track:4YuGhuEypxkqhd3QdewSuk") {

		int i = 1;

		do {
			int err = recordSong(i);
			while (err > 0) {
				logline("Recording Failed, errcode: " + to_string(err), true);
				do {
					logline("reset", true);
					while (init(uri, templog) > 0)
						;
					logline("init success!", true);
				} while (playlist.seek(i) != 0);
				logline("seek complete", true);

				err = recordSong(i);
			}
			i++;
		} while (!playlist.isFinished());
	}

	killSpotify();
	std::string killdisplay = execToString(
			"tightvncserver -kill " + variables::instance().getDisplay());
	logline(killdisplay, true);

//Continue Syncthing
//execToString("kill -s CONT syncthing");
	logline("Finish", true);
}
