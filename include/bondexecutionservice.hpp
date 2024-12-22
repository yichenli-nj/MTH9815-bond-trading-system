#pragma once

#include "executionservice.hpp"
#include "soa.hpp"
#include "products.hpp"
#include <vector>
#include <map>
#include <string>


class BondExcutionServiceConnector;

class BondExecutionService : public ExecutionService<Bond>
{
public:

public:
	BondExecutionService();

	// Get data on our service given a key
	virtual ExecutionOrder<Bond>& GetData(std::string key);

	// Get connector
	virtual BondExecutionServiceConnector& GetConnector() const;

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(ExecutionOrder<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<ExecutionOrder<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector<ServiceListener<ExecutionOrder<Bond>>*>& GetListeners() const;

	// Execute an order on a market
	virtual void ExecuteOrder(ExecutionOrder<Bond>& order, Market market);

	virtual void AlgoExecutionAdd(const AlgoExecution<Bond>& algoExecution);

private:
	std::map<std::string, ExecutionOrder<Bond>> executionorderMap;
	std::vector<ServiceListener<ExecutionOrder<Bond>>*> listeners;
	BondExecutionServiceConnector* connector;

};


class BondExecutionServiceConnector : public Connector<ExecutionOrder<Bond>>
{
public:

	virtual void Publish(ExecutionOrder<Bond>& executionOrder);

	virtual void Subscribe(std::string filePath);
};

class BondExecutionServiceListener : public ServiceListener<AlgoExecution<Bond>>
{
public:
    BondExecutionServiceListener(BondExecutionService* _service);

    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoExecution<Bond>& data);

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoExecution<Bond>& data);

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoExecution<Bond>& data);

private:
    BondExecutionService* service;
};


BondExecutionServiceListener::BondExecutionServiceListener(BondExecutionService* service) :
    service{service}
{
}

// Listener callback to process an add event to the Service
void BondExecutionServiceListener::ProcessAdd(AlgoExecution<Bond>& data)
{
    service->AlgoExecutionAdd(data);
}

void BondExecutionServiceListener::ProcessRemove(AlgoExecution<Bond>& data)
{
}

void BondExecutionServiceListener::ProcessUpdate(AlgoExecution<Bond>& data)
{
}


void BondExecutionServiceConnector::Publish(ExecutionOrder<Bond>& executionOrder)
{
	std::cout << "Order Executed\n";
}

void BondExecutionServiceConnector::Subscribe(std::string filePath)
{
}

BondExecutionService::BondExecutionService() : executionorderMap(std::map<std::string, ExecutionOrder<Bond>>()), listeners(std::vector<ServiceListener<ExecutionOrder<Bond>>*>()), connector(new BondExecutionServiceConnector())
{
}

ExecutionOrder<Bond>& BondExecutionService::GetData(std::string key)
{
	return executionorderMap.at(key);
}

BondExecutionServiceConnector& BondExecutionService::GetConnector() const
{
	return *connector;
}


void BondExecutionService::OnMessage(ExecutionOrder<Bond>& data)
{
}

void BondExecutionService::AddListener(ServiceListener<ExecutionOrder<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector<ServiceListener<ExecutionOrder<Bond>>*>& BondExecutionService::GetListeners() const
{
	return listeners;
}

void BondExecutionService::ExecuteOrder(ExecutionOrder<Bond>& order, Market market)
{
	std::string id = order.GetProduct().GetProductId();
	executionorderMap[id] = order;
	connector->Publish(order);
	for (auto& listener : listeners)
	{
		listener->ProcessAdd(order);
	}
}


void BondExecutionService::AlgoExecutionAdd(const AlgoExecution<Bond>& algoExecution)
{
	ExecutionOrder<Bond> executionOrder = algoExecution.GetExecutionOrder();
	ExecutionOrder<Bond> order = algoExecution.GetExecutionOrder();

	ExecuteOrder(order, BROKERTEC);
}

