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
* SimTest.cpp
*
*  Created on: 22/04/2020
*      Author: Scott Ellis <scott.ellis@novatex.com.au>
*/

#include "../PortLoader.h"
#include <catch.hpp>
#include <opendatacon/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <array>
#include <sstream>

#define SUITE(name) "SimTests - " name

inline Json::Value GetTestConfigJSON()
{
	// We actually have the conf file here to match the tests it is used in below.
	static const char* conf = R"001(
	{
		"HttpIP" : "0.0.0.0",
		"HttpPort" : 9000,
		"Version" : "Dummy Version 2-3-2020",

		//-------Point conf--------#
		"Binaries" :
		[
			{"Index": 0},{"Index": 1},{"Index": 5},{"Index": 6},{"Index": 7},{"Index": 8},{"Index": 10},{"Index": 11},{"Index": 12},{"Index": 13},{"Index": 14},{"Index": 15}
		],

		"Analogs" :
		[
			{"Range" : {"Start" : 0, "Stop" : 2}, "StartVal" : 50, "UpdateIntervalms" : 10000, "StdDev" : 2},
			{"Range" : {"Start" : 3, "Stop" : 5}, "StartVal" : 230, "UpdateIntervalms" : 10000, "StdDev" : 5},
			{"Index" : 7, "StartVal" : 5, "StdDev" : 0} //this is the tap position feedback
			//,{"Index" : 6, "SQLite3" : { "File" : "test2.db", "Query" : "select timestamp,(value+:INDEX) from events", "TimestampHandling" : "RELATIVE_TOD_FASTFORWARD"}}
		],

		"BinaryControls" :
		[
			{
				"Index" : 0,
				"FeedbackBinaries":
				[
					{"Index":0,"FeedbackMode":"LATCH","OnValue":true,"OffValue":false},
					{"Index":1,"FeedbackMode":"LATCH","OnValue":false,"OffValue":true}
				]
			},
			{
				"Index" : 1,
				"FeedbackBinaries":
				[
					{"Index":5,"FeedbackMode":"LATCH","OnValue":true,"OffValue":false},
					{"Index":6,"FeedbackMode":"LATCH","OnValue":false,"OffValue":true}
				]
			},
			{
				"Index" : 2,
				"FeedbackPosition": {"Type": "Analog", "Index" : 7, "FeedbackMode":"PULSE", "Action":"UP", "Limit":10}
			},
			{
				"Index" : 3,
				"FeedbackPosition": {"Type": "Analog", "Index" : 7,"FeedbackMode":"PULSE", "Action":"DOWN", "Limit":0}
			},
			{
				"Index" : 4,
				"FeedbackPosition": {"Type": "Binary", "Indexes" : [10,11,12,13], "FeedbackMode":"PULSE", "Action":"UP", "Limit":10}
			},
			{
				"Index" : 5,
				"FeedbackPosition": {"Type": "Binary", "Indexes" : [10,11,12,13],"FeedbackMode":"PULSE", "Action":"DOWN", "Limit":0}
			},
			{
				"Index" : 6,
				"FeedbackPosition":	{ "Type": "BCD", "Indexes" : [10,11,12,13,14], "FeedbackMode":"PULSE", "Action":"UP", "Limit":10}
			},
			{
				"Index" : 7,
				"FeedbackPosition": {"Type": "BCD", "Indexes" : [10,11,12,13,14],"FeedbackMode":"PULSE", "Action":"DOWN", "Limit":0}
			}
		]
	})001";

	std::istringstream iss(conf);
	Json::CharReaderBuilder JSONReader;
	std::string err_str;
	Json::Value json_conf;
	bool parse_success = Json::parseFromStream(JSONReader,iss, &json_conf, &err_str);
	if (!parse_success)
	{
		FAIL("Failed to parse configuration : " + err_str);
	}
	return json_conf;
}

void TestSetup(spdlog::level::level_enum loglevel)
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto pLibLogger = std::make_shared<spdlog::logger>("SimPort", console_sink);
	pLibLogger->set_level(loglevel);
	odc::spdlog_register_logger(pLibLogger);

	// We need an opendatacon logger to catch config file parsing errors
	auto pODCLogger = std::make_shared<spdlog::logger>("opendatacon", console_sink);
	pODCLogger->set_level(loglevel);
	odc::spdlog_register_logger(pODCLogger);

	static std::once_flag once_flag;
	std::call_once(once_flag,[]()
		{
			InitLibaryLoading();
		});
}

void TestTearDown()
{
	odc::spdlog_drop_all(); // Close off everything
}

ParamCollection BuildParams(const std::string& a,
	const std::string& b,
	const std::string& c)
{
	ParamCollection params;
	params["0"] = a;
	params["1"] = b;
	params["2"] = c;
	params["Target"] = "OutstationUnderTest";
	return params;
}

TEST_CASE("TestConfigLoad")
{
	TestSetup(spdlog::level::level_enum::warn);

	//Load the library
	auto port_lib = LoadModule(GetLibFileName("SimPort"));
	REQUIRE(port_lib);

	//scope for port, ios lifetime
	{
		auto IOS = odc::asio_service::Get();
		newptr new_sim = GetPortCreator(port_lib, "Sim");
		REQUIRE(new_sim);
		delptr delete_sim = GetPortDestroyer(port_lib, "Sim");
		REQUIRE(delete_sim);

		auto sim_port = std::shared_ptr<DataPort>(new_sim("OutstationUnderTest", "", GetTestConfigJSON()), delete_sim);
		sim_port->Build();
		sim_port->Enable();
		const bool result = sim_port->GetCurrentState()["AnalogCurrent"].isMember("0");
		REQUIRE(result == true);
	}

	UnLoadModule(port_lib);
	TestTearDown();
}

TEST_CASE("TestForcedPoint")
{
	//Load the library
	auto port_lib = LoadModule(GetLibFileName("SimPort"));
	REQUIRE(port_lib);

	//scope for port, ios lifetime
	{
		auto IOS = odc::asio_service::Get();
		newptr new_sim = GetPortCreator(port_lib, "Sim");
		REQUIRE(new_sim);
		delptr delete_sim = GetPortDestroyer(port_lib, "Sim");
		REQUIRE(delete_sim);

		auto sim_port = std::shared_ptr<DataPort>(new_sim("OutstationUnderTest", "", GetTestConfigJSON()), delete_sim);

		sim_port->Build();
		sim_port->Enable();

		std::shared_ptr<IUIResponder> resp = std::get<1>(sim_port->GetUIResponder());
		const ParamCollection params = BuildParams("Analog", "0", "12345.6789");
		Json::Value value = resp->ExecuteCommand("ForcePoint", params);
		REQUIRE(value["RESULT"].asString() == "Success");
		REQUIRE(sim_port->GetCurrentState()["AnalogCurrent"]["0"] == "12345.678900");
	}

	UnLoadModule(port_lib);
	TestTearDown();
}

TEST_CASE("TestReleasePoint")
{
	//Load the library
	auto port_lib = LoadModule(GetLibFileName("SimPort"));
	REQUIRE(port_lib);

	//scope for port, ios lifetime
	{
		auto IOS = odc::asio_service::Get();
		newptr new_sim = GetPortCreator(port_lib, "Sim");
		REQUIRE(new_sim);
		delptr delete_sim = GetPortDestroyer(port_lib, "Sim");
		REQUIRE(delete_sim);

		auto sim_port = std::shared_ptr<DataPort>(new_sim("OutstationUnderTest", "", GetTestConfigJSON()), delete_sim);

		sim_port->Build();
		sim_port->Enable();

		std::shared_ptr<IUIResponder> resp = std::get<1>(sim_port->GetUIResponder());
		const ParamCollection params = BuildParams("Analog", "0", "");
		Json::Value value = resp->ExecuteCommand("ReleasePoint", params);
		REQUIRE(value["RESULT"].asString() == "Success");
	}

	UnLoadModule(port_lib);
	TestTearDown();
}

TEST_CASE("TestLatchOn")
{
	//Load the library
	auto port_lib = LoadModule(GetLibFileName("SimPort"));
	REQUIRE(port_lib);

	//scope for port, ios lifetime
	{
		auto IOS = odc::asio_service::Get();
		newptr new_sim = GetPortCreator(port_lib, "Sim");
		REQUIRE(new_sim);
		delptr delete_sim = GetPortDestroyer(port_lib, "Sim");
		REQUIRE(delete_sim);

		auto sim_port = std::shared_ptr<DataPort>(new_sim("OutstationUnderTest", "", GetTestConfigJSON()), delete_sim);
		sim_port->Build();
		sim_port->Enable();

		std::string result = sim_port->GetCurrentState()["BinaryCurrent"]["0"].asString();
		REQUIRE(result == "0");

		// Set up a callback for the result
		std::atomic_bool executed(false);
		CommandStatus cb_status;
		auto pStatusCallback = std::make_shared<std::function<void (CommandStatus status)>>([&cb_status,&executed](CommandStatus status)
			{
				cb_status = status;
				executed = true;
			});

		EventTypePayload<EventType::ControlRelayOutputBlock>::type val;
		val.functionCode = ControlCode::LATCH_ON;
		auto event = std::make_shared<EventInfo>(EventType::ControlRelayOutputBlock, 0);
		event->SetPayload<EventType::ControlRelayOutputBlock>(std::move(val));

		sim_port->Event(event, "TestHarness", pStatusCallback);
		while(!executed)
		{
			IOS->run_one();
		}
		result = sim_port->GetCurrentState()["BinaryCurrent"]["0"].asString();
		REQUIRE(result == "1");
		REQUIRE(cb_status == CommandStatus::SUCCESS);
	}

	UnLoadModule(port_lib);
	TestTearDown();
}

TEST_CASE("TestLatchOff")
{
	//Load the library
	auto port_lib = LoadModule(GetLibFileName("SimPort"));
	REQUIRE(port_lib);

	//scope for port, ios lifetime
	{
		auto IOS = odc::asio_service::Get();
		newptr new_sim = GetPortCreator(port_lib, "Sim");
		REQUIRE(new_sim);
		delptr delete_sim = GetPortDestroyer(port_lib, "Sim");
		REQUIRE(delete_sim);

		auto sim_port = std::shared_ptr<DataPort>(new_sim("OutstationUnderTest", "", GetTestConfigJSON()), delete_sim);
		sim_port->Build();
		sim_port->Enable();

		std::string result = sim_port->GetCurrentState()["BinaryCurrent"]["0"].asString();
		REQUIRE(result == "0");

		// Set up a callback for the result
		std::atomic_bool executed(false);
		CommandStatus cb_status;
		auto pStatusCallback = std::make_shared<std::function<void (CommandStatus status)>>([&cb_status,&executed](CommandStatus status)
			{
				cb_status = status;
				executed = true;
			});

		EventTypePayload<EventType::ControlRelayOutputBlock>::type val;
		val.functionCode = ControlCode::TRIP_PULSE_ON;
		auto event = std::make_shared<EventInfo>(EventType::ControlRelayOutputBlock, 0);
		event->SetPayload<EventType::ControlRelayOutputBlock>(std::move(val));

		sim_port->Event(event, "TestHarness", pStatusCallback);
		while(!executed)
		{
			IOS->run_one();
		}
		result = sim_port->GetCurrentState()["BinaryCurrent"]["0"].asString();
		REQUIRE(result == "0");
		REQUIRE(cb_status == CommandStatus::SUCCESS);
	}

	UnLoadModule(port_lib);
	TestTearDown();
}
