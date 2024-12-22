#pragma once

#include "products.hpp"
#include "pricingservice.hpp"
#include "soa.hpp"
#include <string>
#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>


template <typename T>
class GUIService : public Service<std::string, Price<T>>
{
public:

    // Get data on our service given a key
    Price<T>& GetData(string key);

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T>& data) 

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Price<T>> *listener) ;

    // Get all listeners on the Service.
    const vector< ServiceListener<Price<T>>* >& GetListeners() const;


private:
    map<string, Price<T>> priceMap; 
    vector<ServiceListener<Price<T>>*> listeners; 

};



template <typename T>
void GUIService<T>::OnMessage(Price<T>& data)
{

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    auto timer = std::chrono::system_clock::to_time_t(now);
    
    std::tm bt = *std::localtime(&timer);
    
    std::cout << "[" << std::put_time(&bt, "%H:%M:%S") 
              << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] "
              << data.GetProduct().GetProductId() << " : "
              << data.GetMid() - data.GetBidOfferSpread() * 0.5 << " @ " << data.GetMid() + data.GetBidOfferSpread() * 0.5 << "\n";


}





