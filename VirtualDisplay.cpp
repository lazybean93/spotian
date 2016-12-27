/*
 * Display.cpp
 *
 *  Created on: 25.12.2016
 *      Author: oliver
 */

#include <VirtualDisplay.h>

VirtualDisplay::VirtualDisplay() {
	std::string out = execToString("tightvncserver 2>&1");
	std::vector<std::string> contentLines = split(out, "\n");
	contentLines = split(contentLines.at(0), ":");
	display = ":" + contentLines.at(1);
	setenv("DISPLAY", display.c_str(), true);
}
std::string VirtualDisplay::getVirtualDisplay(){
	return display;
}
VirtualDisplay::~VirtualDisplay() {
	std::string killdisplay = execToString(
			"tightvncserver -kill " + display);
}
