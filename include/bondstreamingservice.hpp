#pragma once

#include "streamingservice.hpp"
#include "bondalgostreamingservice.hpp"
#include "soa.hpp"
#include "products.hpp"
#include <map>
#include <vector>
#include <string>

class BondStreamingServiceConnector;

class BondStreamingService : public StreamingService<Bond>
{
public:
	BondStreamingService();

	// Get data on our service given a key
	virtual PriceStream<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(PriceStream<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<PriceStream<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector<ServiceListener<PriceStream<Bond>>*>& GetListeners() const;
	
	virtual void PublishPrice(PriceStream<Bond>& priceStream);

	virtual void AlgoStreamAdd(const AlgoStream<Bond>& price);

	BondStreamingServiceConnector* connector;
private:
	std::map<std::string, PriceStream<Bond>> pricestreamMap;
	std::vector<ServiceListener<PriceStream<Bond>>*> listeners;
};


class BondStreamingServiceConnector : public Connector<PriceStream<Bond>>
{
public:

	virtual void Publish(PriceStream<Bond>& orderBook);

	virtual void Subscribe(std::string filePath);

private:
};

class BondStreamingServiceListener : public ServiceListener<AlgoStream<Bond>>
{
public:
    BondStreamingServiceListener(BondStreamingService* service);

    // Listener callback to process an add event to the Service
    virtual void ProcessAdd(AlgoStream<Bond>& data);

    // Listener callback to process a remove event to the Service
    virtual void ProcessRemove(AlgoStream<Bond>& data);

    // Listener callback to process an update event to the Service
    virtual void ProcessUpdate(AlgoStream<Bond>& data);

private:
    BondStreamingService* service;
};


BondStreamingServiceListener::BondStreamingServiceListener(BondStreamingService* service) :
    service{service}
{

}

void BondStreamingServiceListener::ProcessAdd(AlgoStream<Bond>& data)
{
    service->AlgoStreamAdd(data);
}

void BondStreamingServiceListener::ProcessRemove(AlgoStream<Bond>& data)
{

}

void BondStreamingServiceListener::ProcessUpdate(AlgoStream<Bond>& data)
{
}


void BondStreamingServiceConnector::Publish(PriceStream<Bond>& priceStream)
{
	std::cout << "Streaming service print!\n";
}

void BondStreamingServiceConnector::Subscribe(std::string filePath)
{
}

BondStreamingService::BondStreamingService(){
}

PriceStream<Bond>& BondStreamingService::GetData(std::string key)
{
	return pricestreamMap.at(key);
}

void BondStreamingService::OnMessage(PriceStream<Bond>& data)
{
}

void BondStreamingService::AddListener(ServiceListener<PriceStream<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector<ServiceListener<PriceStream<Bond>>*>& BondStreamingService::GetListeners() const
{
	return listeners;
}


void BondStreamingService::PublishPrice(PriceStream<Bond>& priceStream)
{
	connector->Publish(priceStream);
}

void BondStreamingService::AlgoStreamAdd(const AlgoStream<Bond>& price)
{
	auto priceStream = price.GetPriceStream();
	auto product = priceStream.GetProduct();
	auto id = product.GetProductId();
	pricestreamMap[id] = priceStream;
	PublishPrice(priceStream);
	for (auto& listener : listeners)
	{
		listener->ProcessAdd(priceStream); 
	}
}


