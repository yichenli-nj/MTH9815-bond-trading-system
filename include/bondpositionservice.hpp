#pragma once

#include "positionservice.hpp"
#include "products.hpp"


class BondPositionService : public PositionService<Bond>
{
public:
	BondPositionService();

	// Get data on our service given a key
	virtual Position<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Position<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<Position<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector< ServiceListener<Position<Bond>>*>& GetListeners() const;

	// Add trade
	virtual void AddTrade(Trade<Bond>& trade);

private:
	std::map<std::string, Position<Bond>> positionMap;
	std::vector<ServiceListener<Position<Bond>>*> listeners;
};


class BondPositionServiceListener : public ServiceListener<Trade<Bond>>
{
public:
    BondPositionServiceListener(BondPositionService* service);

    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(Trade<Bond>& data);

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(Trade<Bond>& data);

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(Trade<Bond>& data);

private:
    BondPositionService* service;
};

BondPositionServiceListener::BondPositionServiceListener(BondPositionService* service) :
    service{service}
{
}

void BondPositionServiceListener::ProcessAdd(Trade<Bond>& data)
{
    service->AddTrade(data);
}

void BondPositionServiceListener::ProcessRemove(Trade<Bond>& data)
{
}

void BondPositionServiceListener::ProcessUpdate(Trade<Bond>& data)
{
}


BondPositionService::BondPositionService()
{
}

Position<Bond>& BondPositionService::GetData(std::string key)
{
	return positionMap.at(key);
}

void BondPositionService::OnMessage(Position<Bond>& data)
{
}

void BondPositionService::AddListener(ServiceListener<Position<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector<ServiceListener<Position<Bond>>*>& BondPositionService::GetListeners() const
{
	return listeners;
}


void BondPositionService::AddTrade(Trade<Bond>& trade)
{
	string book = trade.GetBook();
	long quantity = trade.GetQuantity();
	quantity *= (trade.GetSide() == BUY ? 1 : -1);

	string id = trade.GetProduct().GetProductId();

    positionMap[id].AddPosition(book, quantity);

	for (auto& listener : listeners)
	{
		listener->ProcessAdd(positionMap[id]);
	}
}

