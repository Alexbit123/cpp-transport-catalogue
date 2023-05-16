#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "tests.h"

#include <iostream>
#include <fstream>

int main() {
	/*int query_count_in, query_count_out;
	std::vector<std::string> query;
	std::string str;*/
	transport_catalogue::TransportCatalogue catalogue;


	input_reader::СatalogFilling(std::cin, catalogue);
	stat_reader::RequestProcessing(std::cin, std::cout, catalogue);
	/*int variant;
	std::cin >> variant;
	if (variant == 1) {
		std::ifstream in("tsC_case1_input.txt");

		if (in.is_open()) {
			in >> query_count_in;

			for (int i = 0; i < query_count_in; ++i) {
				std::getline(in >> std::ws, str);
				query.push_back(str);
			}

			input_reader::add_query::Add(query, catalogue);
			query.clear();

			in >> query_count_out;

			for (int i = 0; i < query_count_out; ++i) {
				std::getline(in >> std::ws, str);
				query.push_back(str);
			}

			stat_reader::statistic::PrintResult(query, catalogue);
		}
	}
	else {*/
		/*std::cin >> query_count_in;

		for (int i = 0; i < query_count_in; ++i) {
			std::getline(std::cin >> std::ws, str);
			query.push_back(str);
		}

		input_reader::add_query::Add(query, catalogue);
		query.clear();

		std::cin >> query_count_out;

		for (int i = 0; i < query_count_out; ++i) {
			std::getline(std::cin >> std::ws, str);
			query.push_back(str);
		}

		stat_reader::statistic::PrintResult(query, catalogue);*/
	/*}*/

}
