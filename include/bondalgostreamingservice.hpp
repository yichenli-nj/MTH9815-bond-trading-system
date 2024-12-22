#pragma once

#include "streamingservice.hpp"
#include "soa.hpp"
#include "pricingservice.hpp"
#include "products.hpp"
#include <map>
#include <vector>
#include <string>


template <typename T>
class AlgoStream
{
public:
    AlgoStream(PriceStream<T> priceStream): priceStream{priceStream}
    {}

    PriceStream<T> GetPriceStream() const
    {
        return priceStream;
    }

    void UpdatePriceStream(const Price<T>& price)
    {
        auto midPrice = price.GetMid();
        auto spread = price.GetBidOfferSpread();
        auto bidPrice = midPrice - spread * 0.5;
        auto askPrice = midPrice + spread * 0.5;

        auto VisibleQuantity = (streamCount % 2 ? 1000000 : 2000000);
        auto HiddenQuantity = 2 * VisibleQuantity;
        priceStream = PriceStream<T>{
            price.GetProduct(), 
            PriceStream{bidPrice, VisibleQuantity, HiddenQuantity, BID},
            PriceStream{askPrice, VisibleQuantity, HiddenQuantity, OFFER}
        };
    }

private:
    PriceStream<T> priceStream;
    long streamCount;
};


template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{
public:
    virtual void AddPrice(const Price<T>& price) = 0;
};


class BondAlgoStreamingService : public AlgoStreamingService<Bond>
{
public:
	BondAlgoStreamingService();

	// Get data on our service given a key
	virtual AlgoStream<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(AlgoStream<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<AlgoStream<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const vector<ServiceListener<AlgoStream<Bond>>*>& GetListeners() const;

	// when listener gets a new Price
	virtual void AddPrice(const Price<Bond>& price);

private:
	std::map<std::string, AlgoStream<Bond>> algostreamMap;
	std::vector<ServiceListener<AlgoStream<Bond>>*> listeners;
};


BondAlgoStreamingService::BondAlgoStreamingService() 
{	
}

AlgoStream<Bond>& BondAlgoStreamingService::GetData(std::string key)
{
	return algostreamMap.at(key);
}

void BondAlgoStreamingService::OnMessage(AlgoStream<Bond>& data)
{
}

void BondAlgoStreamingService::AddListener(ServiceListener<AlgoStream<Bond>>* listener)
{
	listeners.push_back(listener);
}

const vector< ServiceListener<AlgoStream<Bond>>*>& BondAlgoStreamingService::GetListeners() const
{
	return listeners;
}

void BondAlgoStreamingService::AddPrice(const Price<Bond>& price)
{
	Bond product = price.GetProduct();
	std::string id = product.GetProductId();
	
	PriceStream<Bond> priceStream(product, PriceStreamOrder(0, 0, 0, BID), PriceStreamOrder(0, 0, 0, OFFER));
	AlgoStream<Bond> algoStream(priceStream);
	algoStream.UpdatePriceStream(price);
	algostreamMap[id] = algoStream;

	for (auto& listener : listeners)
	{
		listener->ProcessAdd(algoStream); // tell BondStreamingServiceListener
	}

}


