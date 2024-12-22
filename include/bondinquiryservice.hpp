#pragma once

#include "soa.hpp"
#include "products.hpp"
#include "tradebookingservice.hpp"
#include "inquiryservice.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

class BondInquiryServiceConnector;


class BondInquiryService : public InquiryService<Bond>
{
public:
	BondInquiryService();

	// Get data on our service given a key
	virtual Inquiry<Bond>& GetData(std::string key);

	// The callback that a Connector should invoke for any new or updated data
	virtual void OnMessage(Inquiry<Bond>& data);

	// Add a listener to the Service for callbacks on add, remove, and update events
	// for data to the Service.
	virtual void AddListener(ServiceListener<Inquiry<Bond>>* listener);

	// Get all listeners on the Service.
	virtual const std::vector<ServiceListener<Inquiry<Bond>>*>& GetListeners() const;

	// Send a quote back to the client
	virtual void SendQuote(const string& inquiryId, double price);

	// Reject an inquiry from the client
	virtual void RejectInquiry(const string& inquiryId);

	BondInquiryServiceConnector* connector;

private:
	std::map<std::string, Inquiry<Bond>> inquiryMap;
	std::vector<ServiceListener<Inquiry<Bond>>*> listeners;
};


class BondInquiryServiceConnector : public Connector<Inquiry<Bond>>
{
public:
	BondInquiryServiceConnector(BondInquiryService* service);

	virtual void Publish(Inquiry<Bond>& inquiry);

	virtual void Subscribe(std::string filePath);

private:
	BondInquiryService* service;
};


BondInquiryService::BondInquiryService()
{
}

Inquiry<Bond>& BondInquiryService::GetData(std::string key)
{
	return inquiryMap.at(key);
}

void BondInquiryService::OnMessage(Inquiry<Bond>& data)
{
	std::string id = data.GetInquiryId(); 
	inquiryMap[id] = data;

	switch (data.GetState())
	{
	case RECEIVED:
		data.SetPrice(100.0);
		connector->Publish(data);
		for (auto& listener : listeners)
		{
			listener->ProcessAdd(data);
		}
		break;
	case QUOTED:
		data.SetState(DONE); // change state from quoted to done
		break;
	default:
		break;
	}

	
}

void BondInquiryService::AddListener(ServiceListener<Inquiry<Bond>>* listener)
{
	listeners.push_back(listener);
}

const std::vector<ServiceListener<Inquiry<Bond>>*>& BondInquiryService::GetListeners() const
{
	return listeners;
}


void BondInquiryService::SendQuote(const string& inquiryId, double price)
{
	auto it = inquiryMap.find(inquiryId);
	if (it == inquiryMap.end()) return;
	it->second.SetPrice(price);

	for (auto& listener : listeners)
	{
		listener->ProcessAdd(it->second);
	}
}

// Reject an inquiry from the client
void BondInquiryService::RejectInquiry(const string& inquiryId)
{
	auto it = inquiryMap.find(inquiryId);
	if (it == inquiryMap.end()) return; 
	it->second.SetState(REJECTED);
	
	for (auto& listener : listeners)
	{
		listener->ProcessAdd(it->second);
	}
}


BondInquiryServiceConnector::BondInquiryServiceConnector(BondInquiryService* service) :
	service{service}
{

}

void BondInquiryServiceConnector::Publish(Inquiry<Bond>& inquiry)
{
	inquiry.SetState(QUOTED);
	service->OnMessage(inquiry);
}


void BondInquiryServiceConnector::Subscribe(std::string filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filePath);
	}

	std::string line;
	int rowNum = 0;

	while (std::getline(file, line)) {
		std::vector<std::string> row = parse(line, ',');
		auto id = row[0];
		auto cusip = row[1];
		auto product = getProduct(cusip);
		auto side = (row[2] == "BUY" ? BUY : SELL);
		auto side = row[2];
		auto quantity = std::stoi(row[3]);
		auto price = getFraction(row[4]);
		InquiryState inquiryState;

		if (row[5] == "RECEIVED")
		{
			inquiryState = RECEIVED;
		}
		else if (row[5] == "QUOTED")
		{
			inquiryState = QUOTED;
		}
		else if (row[5] == "DONE")
		{
			inquiryState = DONE;
		}
		else if (row[5] == "REJECTED")
		{
			inquiryState = REJECTED;
		}
		else if (row[5] == "CUSTOMER_REJECTED")
		{
			inquiryState = CUSTOMER_REJECTED;
		}
		else
		{
			throw std::runtime_error("Invalid Inquiry State");
		}
		
		auto inquiry = Inquiry<Bond>{id, product, side, quantity, price, inquiryState};
		service->OnMessage(inquiry);
		
		
	}
	file.close();

}

