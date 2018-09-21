/*	opendatacon
*
*	Copyright (c) 2018:
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
* MD3Port.cpp
*
*  Created on: 01/04/2018
*      Author: Scott Ellis <scott.ellis@novatex.com.au>
*/


#include <iostream>
#include "MD3Port.h"
#include "MD3PortConf.h"

MD3Port::MD3Port(const std::string &aName, const std::string & aConfFilename, const Json::Value & aConfOverrides):
	DataPort(aName, aConfFilename, aConfOverrides),
	pConnection(nullptr)
{
	//TODO: Do we have to create our own logger instance? Or just assume will be run on linux...
	//md3logger = spdlog::get("MD3Port"); // Only gets the opendatacon logger in Linux at the moment!

	//the creation of a new MD3PortConf will get the point details
	pConf.reset(new MD3PortConf(ConfFilename, ConfOverrides));

	// Just to save a lot of dereferencing..
	MyConf = static_cast<MD3PortConf*>(this->pConf.get());
	MyPointConf = MyConf->pPointConf;

	//We still may need to process the file (or overrides) to get Addr details:
	ProcessFile();
}

TCPClientServer MD3Port::ClientOrServer()
{
	if (MyConf->mAddrConf.ClientServer == TCPClientServer::DEFAULT)
		return TCPClientServer::SERVER;
	return MyConf->mAddrConf.ClientServer;
}
bool MD3Port::IsServer()
{
	return (TCPClientServer::SERVER == ClientOrServer());
}

void MD3Port::ProcessElements(const Json::Value& JSONRoot)
{
	// The points are handled by the MD3PointConf class.
	// This is all the port specific stuff.

	if (!JSONRoot.isObject()) return;

	if (JSONRoot.isMember("IP") && JSONRoot.isMember("SerialDevice"))
		LOGERROR("Warning: MD3 port serial device AND IP address specified - IP overrides");

	if (JSONRoot.isMember("IP"))
	{
		static_cast<MD3PortConf*>(pConf.get())->mAddrConf.IP = JSONRoot["IP"].asString();
	}

	if (JSONRoot.isMember("Port"))
		static_cast<MD3PortConf*>(pConf.get())->mAddrConf.Port = numeric_cast<uint16_t>(JSONRoot["Port"].asUInt());

	if (JSONRoot.isMember("TCPClientServer"))
	{
		if (JSONRoot["TCPClientServer"].asString() == "CLIENT")
			static_cast<MD3PortConf*>(pConf.get())->mAddrConf.ClientServer = TCPClientServer::CLIENT;
		else if (JSONRoot["TCPClientServer"].asString() == "SERVER")
			static_cast<MD3PortConf*>(pConf.get())->mAddrConf.ClientServer = TCPClientServer::SERVER;
		else if (JSONRoot["TCPClientServer"].asString() == "DEFAULT")
			static_cast<MD3PortConf*>(pConf.get())->mAddrConf.ClientServer = TCPClientServer::DEFAULT;
		else
			LOGERROR("Warning: Invalid TCP client/server type, it should be CLIENT, SERVER, or DEFAULT : "+ JSONRoot["TCPClientServer"].asString());
	}

	if (JSONRoot.isMember("OutstationAddr"))
		static_cast<MD3PortConf*>(pConf.get())->mAddrConf.OutstationAddr = numeric_cast<uint8_t>(JSONRoot["OutstationAddr"].asUInt());

	if (JSONRoot.isMember("TCPConnectRetryPeriodms"))
		static_cast<MD3PortConf*>(pConf.get())->mAddrConf.TCPConnectRetryPeriodms = numeric_cast<uint16_t>(JSONRoot["TCPConnectRetryPeriodms"].asUInt());

}

int MD3Port::Limit(int val, int max)
{
	return val > max ? max : val;
}
uint8_t MD3Port::Limit(uint8_t val, uint8_t max)
{
	return val > max ? max : val;
}


void MD3Port::SetSendTCPDataFn(std::function<void(std::string)> Send)
{
	SendTCPDataFn = Send;
}

// Test only method for simulating input from the TCP Connection.
void MD3Port::InjectSimulatedTCPMessage(buf_t&readbuf)
{
	// Just pass to the Connection ReadCompletionHandler, as if it had come in from the TCP port
	pConnection->ReadCompletionHandler(readbuf);
}

// The only method that sends to the TCP Socket
void MD3Port::SendMD3Message(const MD3Message_t &CompleteMD3Message)
{
	if (CompleteMD3Message.size() == 0)
	{
		LOGERROR("Tried to send an empty message to the TCP Port");
		return;
	}
	// Turn the blocks into a binary string.
	std::string MD3MessageString;
	for (auto blk : CompleteMD3Message)
	{
		MD3MessageString += blk.ToBinaryString();
	}

	// This is a pointer to a function, so that we can hook it for testing. Otherwise calls the pSockMan Write templated function
	// Small overhead to allow for testing - Is there a better way? - could not hook the pSockMan->Write function and/or another passed in function due to differences between a method and a lambda
	if (SendTCPDataFn != nullptr)
		SendTCPDataFn(MD3MessageString);
	else
	{
		pConnection->Write(MD3MessageString);
	}
}
