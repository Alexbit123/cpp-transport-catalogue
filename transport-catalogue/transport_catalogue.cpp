#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddBus(detail::Query query) {
		buses.push_back(query.bus_struct);
		busname_to_bus[buses.back().bus_name] = &buses.back();

		for (auto stop : buses.back().route) {
			stopname_to_bus[stop->stop_name].push_back(buses.back().bus_name);
		}
	}

	void TransportCatalogue::AddStop(detail::Query query) {
		stops.push_back(query.stop_struct);
		stopname_to_stop[stops.back().stop_name] = &stops.back();
	}

	void TransportCatalogue::AddDistance(detail::Query query) {
		for (auto& [distance, stopname_one, stopname_two] : query.stop_to_stop_distance) {
			stops_distance[{FindStop(stopname_one), FindStop(stopname_two)}] = distance;
		}
	}

	detail::Bus* TransportCatalogue::FindBus(std::string_view name_bus) {
		if (busname_to_bus.count(name_bus)) {
			return busname_to_bus.at(name_bus);
		}
		return nullptr;
	}

	detail::Stop* TransportCatalogue::FindStop(std::string_view name_stop) {
		if (stopname_to_stop.count(name_stop)) {
			return stopname_to_stop.at(name_stop);
		}
		return nullptr;
	}

	std::vector<std::string_view>& TransportCatalogue::GetBuses(std::string_view name_stop) {
		static std::vector<std::string_view> result;
		if (stopname_to_bus.count(name_stop)) {
			return stopname_to_bus.at(name_stop);
		}
		return result;
	}

	uint64_t TransportCatalogue::GetDistance(std::string_view stop_one, std::string_view stop_two) {
		if (stops_distance.count({ FindStop(stop_one), FindStop(stop_two) })) {
			return stops_distance.at({ FindStop(stop_one), FindStop(stop_two) });
		}
		return stops_distance.at({ FindStop(stop_two), FindStop(stop_one) });
	}

	void TransportCatalogue::GetInfoBus(detail::QueryResultBus result) {           //Bus X: R stops on route, U unique stops, L route length, C curvature
		std::cout << "Bus " << result.query_name << ": " << result.count_stops << " stops on route, "
			<< result.unique_count_stops << " unique stops, " << std::setprecision(6)
			<< result.route_length << " route length, " << result.curvature << " curvature" << std::endl;
	}

	void TransportCatalogue::GetInfoStop(detail::QueryResultStop result) {
		std::cout << "Stop " << result.query_name << ": buses ";
		for (std::string_view stop : result.buses) {
			std::cout << stop << " ";
		}
		std::cout << std::endl;
	}
}//close transport_catalogue