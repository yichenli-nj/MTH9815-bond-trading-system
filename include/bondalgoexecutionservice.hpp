#pragma once

#include "soa.hpp"
#include "executionservice.hpp"
#include "products.hpp"
#include <vector>
#include <map>
#include <string>
#include <algorithm>

template <typename T>
class AlgoExecution
{
public:
    // param ctor
    AlgoExecution(ExecutionOrder<T> _executionOrder);
    AlgoExecution(const OrderBook<T>& orderBook);

    // get the execution order
    ExecutionOrder<T> GetExecutionOrder() const;

    // run algorithm on the new orderbook
    void UpdateExecutionOrder(const OrderBook<T>& orderBook);

private:
    ExecutionOrder<T> executionOrder;
    static int numId;
};

template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{

public:
    // update execution order when given an order book
    virtual void AddOrderBook(const OrderBook<T>& orderBook) = 0;

};


class BondAlgoExecutionService : public AlgoExecutionService<Bond>
{
public:
	BondAlgoExecutionService();

	// Get data on our service given a key
	virtual AlgoExecution<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(AlgoExecution<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<AlgoExecution<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const std::vector< ServiceListener<AlgoExecution<Bond>>*>& GetListeners() const;

private:
	std::map<std::string, AlgoExecution<Bond>> algoexecutionMap; 
	std::vector<ServiceListener<AlgoExecution<Bond>>*> listeners;

};

BondAlgoExecutionService::BondAlgoExecutionService() : algoexecutionMap(std::map<std::string, AlgoExecution<Bond>>()), listeners(std::vector<ServiceListener<AlgoExecution<Bond>>*>())
{
}

AlgoExecution<Bond>& BondAlgoExecutionService::GetData(std::string key)
{
	return algoexecutionMap.at(key);
}

void BondAlgoExecutionService::OnMessage(AlgoExecution<Bond>& data)
{
}

void BondAlgoExecutionService::AddListener(ServiceListener<AlgoExecution<Bond>>* listener)
{
	listeners.push_back(listener);
}


const std::vector< ServiceListener<AlgoExecution<Bond>>*>& BondAlgoExecutionService::GetListeners() const
{
	return listeners;
}



