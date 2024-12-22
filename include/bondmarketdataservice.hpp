#pragma once

#include "marketdataservice.hpp"
#include "products.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include "utils.hpp"
#include <string>

class BondMarketDataService : public MarketDataService<Bond>
{
public:
	BondMarketDataService();

	// Get data on our service given a key
	virtual OrderBook<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(OrderBook<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<OrderBook<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector< ServiceListener<OrderBook<Bond>>*>& GetListeners() const;

	// Get the best bid/offer order
    virtual BidOffer GetBestBidOffer(const string& productId);

    // Aggregate the order book
    virtual OrderBook<Bond> AggregateDepth(const string& productId);

private:
	std::map<std::string, OrderBook<Bond>> marketdataMap;
	std::vector<ServiceListener<OrderBook<Bond>>*> listeners;
};


BondMarketDataService::BondMarketDataService()
{
}

OrderBook<Bond>& BondMarketDataService::GetData(std::string key)
{
	return marketdataMap.at(key);
}

void BondMarketDataService::OnMessage(OrderBook<Bond>& data)
{
	std::string id = data.GetProduct().GetProductId();
	marketdataMap[id] = data;
	OrderBook<Bond> aggOrderBook = AggregateDepth(id);

	// call ProcessAdd on all listeners
	for (auto& listener : listeners)
	{
		listener->ProcessAdd(aggOrderBook);
	}
}

void BondMarketDataService::AddListener(ServiceListener<OrderBook<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector< ServiceListener<OrderBook<Bond>>*>& BondMarketDataService::GetListeners() const
{
	return listeners;
}

BidOffer BondMarketDataService::GetBestBidOffer(const string& productId)
{
	OrderBook<Bond> ob = marketdataMap.at(productId);
	auto&& bids = ob.GetBidStack();
	auto&& offers = ob.GetOfferStack();
	Order bestBid = bids[0];
	Order bestOffer = offers[0];

	for (auto& bid : bids)
	{
		if (bid.GetPrice() > bestBid.GetPrice())
		{
			bestBid = bid;
		}
	}
	for (auto& offer : offers)
	{
		if (offer.GetPrice() < bestOffer.GetPrice())
		{
			bestOffer = offer;
		}
	}
	return BidOffer{bestBid, bestOffer};
}

OrderBook<Bond> BondMarketDataService::AggregateDepth(const string& productId)
{
	OrderBook<Bond> ob = marketdataMap.at(productId);
	std::map<double, long> bids, offers; 

	double price;
	long quantity;
	for (auto& bid : ob.GetBidStack())
	{
		price = bid.GetPrice();
		quantity = bid.GetQuantity();
		bids[price] += quantity;
	
	}
	for (auto& offer : ob.GetOfferStack())
	{
		price = offer.GetPrice();
		quantity = offer.GetQuantity();
		offers[price] += quantity;
	}

	std::vector<Order> bids_res, offers_res;
	for (auto& bid : bids) bids_res.push_back(Order{bid.first, bid.second, BID});
	for (auto& offer : offers) offers_res.push_back(Order{offer.first, offer.second, OFFER});
	auto res = OrderBook<Bond>{ob.GetProduct(), bids_res, offers_res};
}



class BondMarketDataServiceConnector : public Connector<OrderBook<Bond>>
{
public:
	// param ctor
	BondMarketDataServiceConnector(BondMarketDataService* service);

	virtual void Publish(OrderBook<Bond>& orderBook);

	virtual void Subscribe(std::string filePath);

private:
	BondMarketDataService* service;
};


BondMarketDataServiceConnector::BondMarketDataServiceConnector(BondMarketDataService* _service) : service{service}
{
}

void BondMarketDataServiceConnector::Publish(OrderBook<Bond>& orderBook)
{
}

void BondMarketDataServiceConnector::Subscribe(std::string filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filePath);
	}

	std::string line;
	int rowNum = 0;
	std::vector<Order> bids, offers;

	while (std::getline(file, line)) {
		std::vector<std::string> row = parse(line, ',');
		auto cusip = row[0];
		auto product = getProduct(cusip);
		auto price = getFraction(row[1]);
		auto quantity = std::stoi(row[2]);
		auto pricingSide = (row[3] == "BID" ? BID : OFFER);

		(pricingSide == BID ? bids : offers).push_back(
			Order{price, quantity, pricingSide}
		);

		if (++rowNum % 10 == 0)
		{
			auto orderBook = OrderBook<Bond>{product, bids, offers};
			service->OnMessage(orderBook);
			bids.clear();
			offers.clear();
		}
		
	}
	file.close();

}




