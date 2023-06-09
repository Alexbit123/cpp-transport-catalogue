#pragma once
#include "transport_catalogue.h"
#include "input_reader.h"

#include <assert.h>

void TestAddStop() {
	transport_catalogue::detail::Query query;
	transport_catalogue::detail::Stop stop;
	transport_catalogue::TransportCatalogue catalogue;
	std::string str = "Stop My ostanovka: 152.45, 486.18";
	
	query.query_name = "Stop";
	stop.stop_name = "My ostanovka";
	stop.coordinates_.lat = 152.45;
	stop.coordinates_.lng = 486.18;
	query.stop_struct = stop;

	catalogue.AddStop(stop);
	auto result = catalogue.FindStop("My ostanovka");
	assert(result->stop_name == "My ostanovka");
	assert(result->coordinates_.lat == 152.45);
	assert(result->coordinates_.lng == 486.18);
}

void TestAddBus() {
	transport_catalogue::detail::Query query;
	transport_catalogue::detail::Stop stop_one;
	transport_catalogue::detail::Stop stop_two;
	transport_catalogue::detail::Bus bus;
	transport_catalogue::TransportCatalogue catalogue;
	std::string str = "Stop My ostanovka: 152.45, 486.18";
	
	stop_one.stop_name = "My ostanovka";
	stop_one.coordinates_.lat = 152.45;
	stop_one.coordinates_.lng = 486.18;

	stop_two.stop_name = "Shiryh";
	stop_two.coordinates_.lat = 15.455;
	stop_two.coordinates_.lng = 4.1878;

	query.query_name = "Bus";
	bus.bus_name = "256";
	bus.route.push_back(&stop_one);
	bus.route.push_back(&stop_two);
	query.bus_struct = bus;

	catalogue.AddBus(bus);
	auto result = catalogue.FindBus("256");
	assert(result->bus_name == "256");
	assert(result->route[0] == &stop_one);
	assert(result->route[1] == &stop_two);
}

void TestInputReaderParseQueryStopFirstIteration() {
	transport_catalogue::detail::Stop query;
	{
		std::string str = "Stop My ostanovka: 152.45, 486.18";

		query = input_reader::detail::ParseQueryStopFirstIteration(str);

		assert(query.stop_name == "My ostanovka");
		assert(query.coordinates_.lat == 152.45);
		assert(query.coordinates_.lng == 486.18);
	}

	{
		std::string str = "Stop     My    ostanovka:    152.45,    486.18";

		query = input_reader::detail::ParseQueryStopFirstIteration(str);

		assert(query.stop_name == "My    ostanovka");
		assert(query.coordinates_.lat == 152.45);
		assert(query.coordinates_.lng == 486.18);
	}

	{
		std::string str = "Stop     My    ostanovka:    152.45,    486.18,   4800m    to   Mariushka";

		query = input_reader::detail::ParseQueryStopFirstIteration(str);

		assert(query.stop_name == "My    ostanovka");
		assert(query.coordinates_.lat == 152.45);
		assert(query.coordinates_.lng == 486.18);
	}
}

void TestInputReaderParseQueryStopSecondIteration() {
	transport_catalogue::detail::Distance query;

	{
		std::string str = "Stop My ostanovka: 152.45, 486.18, 500m to Stir, 480m to Kalinka";

		query = input_reader::detail::ParseQueryStopSecondIteration(str);

		assert(query.stop_to_stop_distance.count(std::tuple("My ostanovka", "Stir", 500)) == 1);
		assert(query.stop_to_stop_distance.count(std::tuple("My ostanovka", "Kalinka", 480)) == 1);
	}

	{
		std::string str = "Stop     My    ostanovka:    152.45,    486.18,   4800m to Mariushka";

		query = input_reader::detail::ParseQueryStopSecondIteration(str);

		assert(query.stop_to_stop_distance.count(std::tuple("My    ostanovka", "Mariushka", 4800)) == 1);
	}
}
