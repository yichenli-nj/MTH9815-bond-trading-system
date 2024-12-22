#pragma once


#include "tradebookingservice.hpp"
#include "products.hpp"
#include "utils.hpp"
#include <sstream>
#include <fstream>
#include <string>


class BondTradeBookingService : public TradeBookingService<Bond>
{
public:
	BondTradeBookingService();

	// Get data on our service given a key
	virtual Trade<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Trade<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<Trade<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector< ServiceListener<Trade<Bond>>*>& GetListeners() const;

	// Book the trade
	void BookTrade(Trade<Bond>& trade);


private:
	std::map<std::string, Trade<Bond>> tradeMap;
	std::vector<ServiceListener<Trade<Bond>>*> listeners;
};

BondTradeBookingService::BondTradeBookingService() : tradeMap(std::map<std::string, Trade<Bond>>()), listeners(std::vector<ServiceListener<Trade<Bond>>*>())
{

}

Trade<Bond>& BondTradeBookingService::GetData(std::string key)
{
	return tradeMap.at(key);
}

void BondTradeBookingService::OnMessage(Trade<Bond>& data)
{
	BookTrade(data);
}

void BondTradeBookingService::AddListener(ServiceListener<Trade<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector<ServiceListener<Trade<Bond>>*>& BondTradeBookingService::GetListeners() const
{
	return listeners;
}

void BondTradeBookingService::BookTrade(Trade<Bond>& trade)
{
	std::string id = trade.GetTradeId();
	tradeMap[id] = trade;

	for (auto& listener : listeners)
	{
		listener->ProcessAdd(trade);
	}
}


class BondTradeBookingServiceConnector : public Connector<Trade<Bond>>
{
public:
	BondTradeBookingServiceConnector(BondTradeBookingService* service);

	virtual void Publish(Connector<Bond>& trade);

	virtual void Subscribe(std::string filePath);

private:
	BondTradeBookingService* service;
};


BondTradeBookingServiceConnector::BondTradeBookingServiceConnector(BondTradeBookingService* service) :
	service{service}
{

}
void BondTradeBookingServiceConnector::Publish(Connector<Bond>& trade)
{

}

void BondTradeBookingServiceConnector::Subscribe(std::string filePath)
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
		auto tradeId = row[1];
		auto price = row[2];
		auto book = row[3];
		auto quantity = row[4];
		auto side = row[5];

		auto trade = Trade<Bond>{
			getProduct(cusip),
			tradeId,
			getFraction(price),
			book,
			std::stoi(quantity),
			side == "BUY" ? BUY : SELL
		};
		service->OnMessage(trade);
		
	}

	file.close();

}




