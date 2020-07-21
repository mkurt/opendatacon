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
 * SimPortPointData.h
 *
 *  Created on: 2020-07-15
 *  The year of bush fires and pandemic
 *      Author: Rakesh Kumar <cpp.rakesh@gmail.com>
 */

#ifndef SIMPORTPOINTDATA_H
#define SIMPORTPOINTDATA_H

#include <opendatacon/IOTypes.h>
#include <vector>
#include <memory>
#include <unordered_map>

/*
  This point is only for interfacing between Point data and sim configuration
  With this interface user can doesn't have to bother about Analog / Binary point distinctions
  Will use Analog Point & Binary point for storage, so we can consume less memory at run time
 */
struct Point
{
	Point(double start_val,
		std::size_t refresh_rate,
		double standard_dev):
		start_value(start_val),
		value(start_val),
		forced_state(false),
		update_interval(refresh_rate),
		std_dev(standard_dev) {}

	double start_value;
	double value;
	bool forced_state;
	// This time interval is in milliseconds.
	std::size_t update_interval;
	// Only required for analog points
	double std_dev;
};

class SimPortPointData
{
public:
	SimPortPointData();

	void SetPoint(const odc::EventType& type, std::size_t index, std::shared_ptr<Point> point);
	void SetForcedState(const odc::EventType& type, std::size_t index, bool state);
	bool GetForcedState(const odc::EventType& type, std::size_t index);
	void SetUpdateInterval(const odc::EventType& type, std::size_t index, std::size_t value);
	std::size_t GetUpdateInterval(const odc::EventType& type, std::size_t index);
	double GetStartValue(const odc::EventType& type, std::size_t index);
	double GetStdDev(std::size_t index);
	void SetValue(const odc::EventType& type, std::size_t index, double value);
	double GetValue(const odc::EventType& type, std::size_t index);

	std::vector<std::size_t> GetIndexes(const odc::EventType& type);
	std::unordered_map<std::size_t , double> GetValues(const odc::EventType& type);

	bool IsIndex(const odc::EventType& type, std::size_t index);

private:
	struct BinaryPoint
	{
		BinaryPoint(bool start_val,
			std::size_t refresh_rate):
			start_value(start_val),
			value(start_val),
			forced_state(false),
			update_interval(refresh_rate) {}

		bool start_value;
		bool value;
		bool forced_state;
		// This time interval is in milliseconds.
		std::size_t update_interval;
	};

	// Using the same Point definition for AnalogPoint as it is same
	using AnalogPoint = Point;
	std::unordered_map<std::size_t, std::shared_ptr<AnalogPoint>> m_analog_points;
	std::unordered_map<std::size_t, std::shared_ptr<BinaryPoint>> m_binary_points;
};

#endif // SIMPORTPOINTDATA_H
