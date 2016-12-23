/*
 * tools.h
 *
 *  Created on: 16.01.2015
 *      Author: oliver
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

std::string boolToStr(bool var);
FILE * popen2(std::string command, std::string type, int & pid);
int pclose2(FILE * fp, pid_t pid);
std::string string_replace(std::string src, std::string const& target, std::string const& repl);
std::string execToString(std::string command);
std::string execDisplayToString(std::string command);
std::vector<std::string> split(std::string org, std::string separator);
void logline(std::string log, bool active);
std::string numToString(float number, int precision = 0);
std::string toUTF8(std::string str);
int strfind(std::string str, std::string query);
std::string decode(std::string str);
int getProcIdByName(std::string procName);
std::string readFile(std::string filename, bool *success = NULL);
double timevalDelta(timeval end, timeval start);
void deleteFile(std::string filename);
void deleteOld(std::string directory, int number);
void deleteFolder(std::string foldername);
void writeToFile(std::string content, std::string file, bool append);
std::string dbus(std::string service, std::string path, std::string method,
		bool *success = NULL);
std::string changeFormat(std::string str);
int getUTime(int pid);
float avgCPULoad(float time);
bool waitLoad(float maxLoad, float time, bool log, float timeout,
		std::string reason = "");
std::string startDisplay();
void mkdir(std::string folder);
bool is_file_exist(std::string fileName);
#endif /* TOOLS_H_ */
