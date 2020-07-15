/*	opendatacon
 *
 *	Copyright (c) 2020:
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
 * SimPortData.h
 *
 *  Created on: 2020-07-15
 *  The year of bush fires and pandemic
 *      Author: Rakesh Kumar <cpp.rakesh@gmail.com>
 */

#ifndef SIMPORTDATA_H
#define SIMPORTDATA_H

#include <string>

class SimPortData
{
public:
	SimPortData();

	void HttpAddress(const std::string& http_addr);
	std::string HttpAddress() const;
	void HttpPort(const std::string& http_port);
	std::string HttpPort() const;
	void Version(const std::string& version);
	std::string Version() const;
	double GetDefaultStdDev() const;

private:
	std::string m_http_addr;
	std::string m_http_port;
	std::string m_version;
	double m_default_std_dev_factor;
};

#endif // SIMPORTDATA_H
