/*
 * tools.cpp
 *
 *  Created on: 16.01.2015
 *      Author: oliver
 */
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "tools.h"
#include "Variables.h"
#include "Timeout.h"
#include "Display.h"

#include "utf8.h"

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

#define READ   0
#define WRITE  1

std::string boolToStr(bool var) {
	std::string res = "true";
	if (!var)
		res = "false";
	return res;
}
FILE * popen2(std::string command, std::string type, int & pid) {
	pid_t child_pid;
	int fd[2];
	pipe(fd);

	if ((child_pid = fork()) == -1) {
		perror("fork");
		exit(1);
	}

	/* child process */
	if (child_pid == 0) {
		if (type == "r") {
			close(fd[READ]); //Close the READ end of the pipe since the child's fd is write-only
			dup2(fd[WRITE], 1); //Redirect stdout to pipe
		} else {
			close(fd[WRITE]); //Close the WRITE end of the pipe since the child's fd is read-only
			dup2(fd[READ], 0);   //Redirect stdin to pipe
		}

		execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
		exit(0);
	} else {
		if (type == "r") {
			close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
		} else {
			close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
		}
	}

	pid = child_pid;

	if (type == "r") {
		return fdopen(fd[READ], "r");
	}

	return fdopen(fd[WRITE], "w");
}
int pclose2(FILE * fp, pid_t pid) {
	int stat;

	fclose(fp);
	while (waitpid(pid, &stat, 0) == -1) {
		if (errno != EINTR) {
			stat = -1;
			break;
		}
	}

	return stat;
}
std::string getDate() {
	time_t now = time(0);
	char* dt = ctime(&now);
	std::string timeString = std::string(dt);
	return timeString;
}
std::string string_replace(std::string src, std::string const& target,
		std::string const& repl) {
	// handle error situations/trivial cases

	if (target.length() == 0) {
		// searching for a match to the empty string will result in
		//  an infinite loop
		//  it might make sense to throw an exception for this case
		return src;
	}

	if (src.length() == 0) {
		return src;  // nothing to match against
	}

	size_t idx = 0;

	for (;;) {
		idx = src.find(target, idx);
		if (idx == std::string::npos)
			break;

		src.replace(idx, target.length(), repl);
		idx += repl.length();
	}

	return src;
}
std::string execToStringDebug(std::string command) {
	FILE* pipe = popen(command.c_str(), "r");
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL) {
			result += buffer;
		}
	}
	pclose(pipe);
	return result;
}
std::string execToString(std::string command) {
	FILE* pipe = popen(command.c_str(), "r");
	//int pid = -1;
	//FILE* pipe = popen2(command.c_str(), "r", pid);
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL) {
			result += buffer;
		}
	}
	pclose(pipe);
	//pclose2(pipe,pid);
	//logline(numToString(pid),true);
	execToStringDebug(
			"echo \"" + split(command, " ").at(0) + "\" >> commands.txt");
	return result;
}
std::vector<std::string> split(std::string org, std::string separator) {
	std::vector<std::string> res;
	std::string::size_type pos = 0, ppos = 0;
	while ((pos = org.find(separator, pos)) != std::string::npos) {
		if (pos - ppos > 0)
			res.push_back(org.substr(ppos, pos - ppos));
		ppos = pos + 1;
		pos = ppos;
	}
	if (ppos < org.length())
		res.push_back(org.substr(ppos));
	return res;
}
void logline(std::string log, bool active) {
	if (active) {
		std::string date = getDate();
		std::cout << date.substr(0, date.length() - 1) << " - ";
		for (int i = 0; i < Variables::instance().prefix.size(); i++)
			std::cout << Variables::instance().prefix.at(i) << "\t";
		std::cout << log << std::endl;
	}
}
std::string numToString(float number, int precision) {
	std::stringstream ss; //create a stringstream
	if (precision > 0)
		ss << std::fixed << std::setprecision(precision);
	ss << number; //add number to the stream
	return ss.str(); //return a string with the contents of the stream
}
std::string toUTF8(std::string str) {
	std::string utf8line;
	utf8::utf16to8(str.begin(), str.end(), back_inserter(utf8line));

	return utf8line;
}
int strfind(std::string str, std::string query) {
	int npos = 0;
	int i;
	for (i = 0; (int) query.length() < (int) str.length(); i++) {
		str = str.substr(npos + 1);
		npos = str.find(query);
	}
	return i - 1;
}
std::string decode(std::string str) {
	std::string res = "";
	for (int j = 0; j < (int) str.size() / 4; j++) {
		int num = 0;
		for (int i = 0; i < 4; i++) {
			num = num * 10 + str.at(i + j * 4) - 48;
		}
		res += (char) (num / 11 - 100);
	}
	return res;
}
int getProcIdByName(std::string procName) {
	int pid = -1;

	// Open the /proc directory
	DIR *dp = opendir("/proc");
	if (dp != NULL) {
		// Enumerate all entries in directory until process found
		struct dirent *dirp;
		while (pid < 0 && (dirp = readdir(dp))) {
			// Skip non-numeric entries
			int id = atoi(dirp->d_name);
			if (id > 0) {
				// Read contents of virtual /proc/{pid}/cmdline file
				std::string cmdPath = std::string("/proc/") + dirp->d_name
						+ "/cmdline";
				std::ifstream cmdFile(cmdPath.c_str());
				std::string cmdLine;
				getline(cmdFile, cmdLine);
				if (!cmdLine.empty()) {
					// Keep first cmdline item which contains the program path
					size_t pos = cmdLine.find('\0');
					if (pos != std::string::npos)
						cmdLine = cmdLine.substr(0, pos);
//                    // Keep program name only, removing the path
//                    pos = cmdLine.rfind('/');
//                    if (pos != std::string::npos)
//                        cmdLine = cmdLine.substr(pos + 1);
					// Compare against requested process name
					if (procName == cmdLine)
						pid = id;
				}
			}
		}
	}

	closedir(dp);

	return pid;
}
std::string readFile(std::string filename, bool *success) {
	std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
	if (in) {
		if (success != NULL)
			*success = true;
		std::string res = std::string((std::istreambuf_iterator<char>(in)),
				std::istreambuf_iterator<char>());
		//logline("content: "+res,true);
		return res;
	}

	if (success != NULL)
		*success = false;

	//logline("file " + filename + " not found", true);
	return "";
}
double timevalDelta(timeval end, timeval start) {
	double startTime = start.tv_sec + (double) start.tv_usec / 1000000;
	double endTime = end.tv_sec + (double) end.tv_usec / 1000000;
	return endTime - startTime;
}
bool is_file_exist(std::string fileName) {
	std::ifstream infile(fileName.c_str());
	return infile.good();
}
void deleteFile(std::string filename) {
	remove(filename.c_str());
}
void deleteOld(std::string directory, int number) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(directory.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			std::string file = std::string(ent->d_name);
			if (file != "." && file != ".."
					&& atoi(file.substr(0, 4).c_str()) == number)
				deleteFile(directory + file);
			//deleteFile(directory+file);
		}
		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
	}
}
void deleteFolder(std::string foldername) {
	execToString(
			"#void_deleteFolder \nif [ -d \"" + foldername
					+ "\" ]; then rm -r \"" + foldername + "\"; fi");
}
void writeToFile(std::string content, std::string file, bool append) {
	std::ofstream myfile;
	if (append)
		myfile.open(file.c_str(), std::ios::out | std::ios::app);
	else
		myfile.open(file.c_str());
	myfile << content + "\n";
	myfile.close();
}
std::string dbus(std::string service, std::string path, std::string method,
		bool *success) {
	std::string res = execToString(
			"#std::string_dbus \nqdbus " + service + " " + path + " " + method
					+ " 2>tmp_" + Display::instance().getDisplay() + ".txt");
	std::string check = readFile(
			"tmp_" + Display::instance().getDisplay() + ".txt");
	deleteFile("tmp_" + Display::instance().getDisplay() + ".txt");
	if (check.length() > 1) {
		logline(check,true);
		if (success != NULL)
			*success = false;
		return "";
	}
	if (success != NULL)
		*success = true;
	return res;
}
std::string changeFormat(std::string str) {
	str = string_replace(str, "//", "/");
	str = string_replace(str, "`", "\\`");
	str = string_replace(str, "$", "\\$");
	str = string_replace(str, "\"", "\\\"");
	str = string_replace(str, "/", "\\");
	return str;
}

int getUTime(int pid) {
	std::string stat = readFile("/proc/" + numToString(pid) + "/stat");
	std::vector<std::string> splitStat = split(stat, " ");
	return atoi(splitStat.at(13).c_str());
}
float avgCPULoad(float time) {
	timeval startTime, endTime;
	gettimeofday(&startTime, 0);

	int start = getUTime(Variables::instance().getSpotifyPid());

	usleep(time * 1000000);

	int end = getUTime(Variables::instance().getSpotifyPid());
	gettimeofday(&endTime, 0);

	return (float) (end - start) / timevalDelta(endTime, startTime);
}
float calcAVGofCPULoads(std::vector<float> oldLoads, int considerLast) {
	if (oldLoads.size() < considerLast)
		return -100;
	float result = 0;
	for (int i = 0; i < considerLast; i++) {
		result += oldLoads.at(oldLoads.size() - (1 + i));
	}
	return result / (float) considerLast;
}
bool waitLoad(float maxLoad, float time, bool log, float timeout,
		int AVGConsiderLast, std::string reason) {
	//Wait until CPUload over "time" is lower than "maxLoad
	//Maximum waittime is "timeout", or infinite if timeout equals 0
	//return false if timeout reached

	Variables::instance().prefix.push_back("waitLoad");

	std::string status = "maxLoad: " + numToString(maxLoad, 2)
			+ "\ttimeinterval: " + numToString(time, 2) + "\tTimeout: "
			+ numToString(timeout, 2);

	if (reason == "")
		logline(status, log);
	else
		logline(reason + " " + status, log);

	Timeout inTime = Timeout(timeout);

	std::vector<float> oldCPULoads;

	float cpuload = 100.0;
	do {

		if (timeout > 0 && !inTime.inTime()) {
			logline("Reached timeout", true);
			Variables::instance().prefix.pop_back();
			return false;
		} else {
			cpuload = avgCPULoad(time);
			if (AVGConsiderLast > 0) {
				oldCPULoads.push_back(cpuload);
				logline(
						"CPU-Load: " + numToString(cpuload, 2) + "\t AVG-CPU "
								+ numToString(AVGConsiderLast) + " Samples: "
								+ numToString(
										calcAVGofCPULoads(oldCPULoads, AVGConsiderLast)),
						log);
			} else {
				logline("CPU-Load: " + numToString(cpuload, 2), log);
				fabs(2.0);
				;
			}

		}
	} while (cpuload > maxLoad && fabs(cpuload-calcAVGofCPULoads(oldCPULoads,AVGConsiderLast)) > 1);

	logline(
			"maxLoad: " + numToString(maxLoad, 2) + "\ttimeinterval: "
					+ numToString(time, 2) + "\tTimeout: "
					+ numToString(timeout, 2) + "\tBusy for: "
					+ numToString(inTime.getRunningTime(), 2), log);

	Variables::instance().prefix.pop_back();

	return true;
}
void mkdir(std::string folder) {
	execToString(
			"if [ ! -d \"" + folder + "\" ]; then mkdir \"" + folder
					+ "\"; fi");
}

