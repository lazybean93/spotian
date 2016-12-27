/*
 * Display.h
 *
 *  Created on: 25.12.2016
 *      Author: oliver
 */

#ifndef VIRTUALDISPLAY_H_
#define VIRTUALDISPLAY_H_

#include <iostream>
#include "tools.h"

class VirtualDisplay {
	std::string display;
	VirtualDisplay();
	VirtualDisplay(const VirtualDisplay&);
	VirtualDisplay & operator =(const VirtualDisplay &);
public:
	static VirtualDisplay& instance() {
		static VirtualDisplay _instance;
		return _instance;
	}
	~VirtualDisplay();
	std::string getVirtualDisplay();
};

#endif /* VIRTUALDISPLAY_H_ */
