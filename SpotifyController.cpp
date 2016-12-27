/*
 * SpotifyController.cpp
 *
 *  Created on: 26.12.2016
 *      Author: oliver
 */

#include <SpotifyController.h>

SpotifyController::SpotifyController() {
	// TODO Auto-generated constructor stub
	mainPage = true;
	spotifyPid = -1;
}
SpotifyController::~SpotifyController() {
	// TODO Auto-generated constructor stub
	killSpotify();
}

void SpotifyController::startSpotify() {
	logline("startSpotify", true);
	killSpotify();

	execToString(
			"rm .config/spotify/Users/lazybean93-user/*.bnk >/dev/null 2>&1 &");
	spotifyPid =
			atoi(
					execToString(
							"spotify 1>/dev/null 2>" + Variables::instance().getTempLogFile()
									+ " & echo $!").c_str());
	mainPage = true;

}

void SpotifyController::killSpotify() {
	bool success = true;
	if (spotifyPid == -1)
		spotifyPid = getProcIdByName("/usr/share/spotify/spotify");
	while (success && spotifyPid != -1) {
		kill(spotifyPid, SIGTERM);
		readFile(
				"/proc/" + numToString(spotifyPid)
						+ "/stat", &success);
		usleep(0.1 * 1000000);
	}
	spotifyPid = -1;
	deleteFolder(Variables::instance().getHomeDir() + "/.cache/spotify");
}

void SpotifyController::setMainPage(bool mainPage) {
	this->mainPage = mainPage;
}
bool SpotifyController::getMainPage() {
	return mainPage;
}
int SpotifyController::getSpotifyPid() {
	return spotifyPid;
}
void SpotifyController::setSpotifyPid(int pid) {
	spotifyPid = pid;
}
