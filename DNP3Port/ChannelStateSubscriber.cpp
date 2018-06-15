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
 *	ChannelStateSubscriber.cpp
 *
 *  Created on: 2016-01-27
 *      Author: Neil Stephens <dearknarl@gmail.com>
 */

#include "ChannelStateSubscriber.h"
#include <utility>

std::multimap<asiodnp3::IChannel*, DNP3Port*> ChannelStateSubscriber::SubscriberMap;
std::mutex ChannelStateSubscriber::MapMutex;

void ChannelStateSubscriber::Subscribe(DNP3Port* pPort, asiodnp3::IChannel* pChan)
{
	std::lock_guard<std::mutex> lock(MapMutex);
	if(SubscriberMap.count(pChan) == 0)
	{
		pChan->AddStateListener([pChan](ChannelState state)
			{
				ChannelStateSubscriber::StateListener(pChan,state);
			});
	}
	SubscriberMap.insert({pChan,pPort});
}
void ChannelStateSubscriber::Unsubscribe(DNP3Port* pPort, asiodnp3::IChannel* pChan)
{
	std::lock_guard<std::mutex> lock(MapMutex);
	if(SubscriberMap.empty())
		return;

	auto bounds = std::make_pair(SubscriberMap.begin(),SubscriberMap.end());
	if(pChan != nullptr)
		bounds = SubscriberMap.equal_range(pChan);
	for(auto aMatch_it = bounds.first; aMatch_it != bounds.second; /*advance inside loop*/)
	{
		if((*aMatch_it).second == pPort)
		{
			SubscriberMap.erase(aMatch_it++); //erase and advance in one go
		}
		else
		{
			++aMatch_it;
		}
	}
}
void ChannelStateSubscriber::StateListener(asiodnp3::IChannel* pChan, ChannelState state)
{
	std::lock_guard<std::mutex> lock(MapMutex);
	auto bounds = SubscriberMap.equal_range(pChan);
	for(auto aMatch_it = bounds.first; aMatch_it != bounds.second; aMatch_it++)
	{
		(*aMatch_it).second->StateListener(state);
	}
}
