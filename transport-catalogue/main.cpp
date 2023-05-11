#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>

int main() {
	int query_count_in, query_count_out;
	std::vector<std::string> query;
	std::string str;
	transport_catalogue::TransportCatalogue catalogue;

	std::cin >> query_count_in;

	for (int i = 0; i < query_count_in; ++i) {
		std::getline(std::cin >> std::ws , str);
		query.push_back(str);
	}

	input_reader::add_query::Add(query, catalogue);
	query.clear();

	std::cin >> query_count_out;

	for (int i = 0; i < query_count_out; ++i) {
		std::getline(std::cin >> std::ws, str);
		query.push_back(str);
	}

	stat_reader::statistic::GetResult(query, catalogue);
	
}