#pragma once


#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "utils.hpp"
#include "soa.hpp"
#include "pricingservice.hpp"
#include "products.hpp"


class BondPricingService : public PricingService<Bond>
{
public:
	BondPricingService();

	// Get data on our service given a key
	virtual Price<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Price<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<Price<Bond>>* listener);
	
	// Get all listeners on the Service.
	virtual const vector< ServiceListener<Price<Bond>>*>& GetListeners() const;


private:
	std::map<std::string, Price<Bond>> priceMap;
	std::vector<ServiceListener<Price<Bond>>*> listeners;
};


BondPricingService::BondPricingService()
{
}

Price<Bond>& BondPricingService::GetData(std::string key)
{
	return priceMap.at(key);
}

void BondPricingService::OnMessage(Price<Bond>& data)
{
	std::string id = data.GetProduct().GetProductId();

	priceMap[id] = data;
	
	for (auto& listener : listeners)
	{
		listener->ProcessAdd(data); 
	}
}

void BondPricingService::AddListener(ServiceListener<Price<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector< ServiceListener<Price<Bond>>*>& BondPricingService::GetListeners() const
{
	return listeners;
}



class BondPricingServiceConnector : public Connector<Price<Bond>>
{
public:
	BondPricingServiceConnector(BondPricingService* service);

	virtual void Publish(Price<Bond>& price);

	virtual void Subscribe(std::string filePath);

private:
	BondPricingService* service;
};


BondPricingServiceConnector::BondPricingServiceConnector(BondPricingService* service):
	service{service}
{
}

void BondPricingServiceConnector::Publish(Price<Bond>& price)
{
}


void BondPricingServiceConnector::Subscribe(std::string filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filePath);
	}

	std::string line;
	int rowNum = 0;
	while (std::getline(file, line)) {
		std::vector<std::string> row = parse(line, ',');
		auto cusip = row[0];
		auto product = getProduct(cusip);
		auto price_mid = getFraction(row[1]);
		auto spread = getFraction(row[2]);

		auto price = Price<Bond>{product, price_mid, spread};
		service->OnMessage(price);
		
	}

	file.close();

}



