#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "products.hpp"
#include "riskservice.hpp"

std::vector<std::string> parse(const std::string& line, char delim) {
	std::vector<std::string> row;
	std::string field;
	std::stringstream ss(line);

	while (std::getline(ss, field, delim)) {
		// Remove any leading/trailing whitespace
		size_t first = field.find_first_not_of(" \t");
		size_t last = field.find_last_not_of(" \t");
		if (first != std::string::npos && last != std::string::npos) {
			field = field.substr(first, (last - first + 1));
		}
		row.push_back(field);
	}

	return row;
}

double getFraction(const std::string& priceString)
{
	auto vecDigits = parse(priceString, '-');
	double integer = std::stoi(vecDigits[0]);
	double digits12 = std::stoi(vecDigits[1].substr(0, 2));
	double digits3 = std::stoi(vecDigits[1].substr(2,3));
	double price = integer + digits12 / 32.0 + digits3 / 256.0;
	return price;
}

Bond getProduct(std::string cusip)
{
	return Bond{
		cusip,
		CUSIP,
		"T",
		1.0,
		boost::gregorian::date(2025, 1, 1)
	};
}

PV01<Bond> getPV01(Bond product)
{
	return {product, 0.1, 1};
}

