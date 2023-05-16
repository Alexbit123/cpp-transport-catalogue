#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddBus(const detail::Bus& query) {
		buses.push_back(query);
		busname_to_bus[buses.back().bus_name] = &buses.back();

		for (auto stop : buses.back().route) {
			stopname_to_bus[stop->stop_name].push_back(buses.back().bus_name);
		}
	}

	void TransportCatalogue::AddStop(const detail::Stop& query) {
		stops.push_back(query);
		stopname_to_stop[stops.back().stop_name] = &stops.back();
	}

	void TransportCatalogue::AddDistance(const detail::Distance& query) {
		for (auto& [stopname_one, stopname_two, distance] : query.stop_to_stop_distance) {
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

	detail::QueryResultBus TransportCatalogue::GetInfoBus(std::string_view name_bus) {           //Bus X: R stops on route, U unique stops, L route length, C curvature
		detail::QueryResultBus result;
		std::unordered_set<transport_catalogue::detail::Stop*> count;
		double route_length_geo = 0;
		result.query_name = name_bus;
		auto bus = this->FindBus(result.query_name);
		if (bus != nullptr) {
			result.count_stops = static_cast<int>(bus->route.size());

			count.insert(bus->route.begin(), bus->route.end());
			result.unique_count_stops = static_cast<int>(count.size());

			for (size_t i = 0; i < bus->route.size() - 1; ++i) {
				result.route_length += this->GetDistance(bus->route[i]->stop_name, bus->route[i + 1]->stop_name);
				route_length_geo += geo_coordinates::distance::ComputeDistance(bus->route[i]->coordinates_,
					bus->route[i + 1]->coordinates_);
			}
			result.curvature = result.route_length / route_length_geo;
		}
		return result;
	}

	detail::QueryResultStop TransportCatalogue::GetInfoStop(std::string_view name_stop) {
		transport_catalogue::detail::QueryResultStop result;
		result.query_name = name_stop;
		if (!GetBuses(result.query_name).empty()) {
			result.buses.insert(GetBuses(result.query_name).begin(), GetBuses(result.query_name).end());
		}
		return result;
	}
}//close transport_catalogue