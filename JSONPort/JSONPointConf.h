/*	opendatacon
 *
 *	Copyright (c) 2014:
 *
 *		DCrip3fJguWgVCLrZFfA7sIGgvx1Ou3fHfCxnrz4svAi
 *		yxeOtDhDCXf1Z4ApgXvX5ahqQmzRfJ2DoX8S05SqHA==
 *	
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *	
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 */ 
/*
 * JSONPointConf.h
 *
 *  Created on: 22/07/2014
 *      Author: Neil Stephens <dearknarl@gmail.com>
 */

#ifndef JSONPOINTCONF_H_
#define JSONPOINTCONF_H_

#include <vector>
#include <unordered_map>
#include <functional>
#include "opendnp3/app/MeasurementTypes.h"
#include "../src/DataPointConf.h"
#include "../src/ConfigParser.h"

class JSONPointConf: public ConfigParser
{
public:
	JSONPointConf(std::string FileName);

	void ProcessElements(const Json::Value& JSONRoot);

	std::map<size_t, Json::Value> Binaries;
	std::map<size_t, Json::Value> Analogs;
	std::map<size_t, Json::Value> Controls;
};

#endif /* JSONPOINTCONF_H_ */