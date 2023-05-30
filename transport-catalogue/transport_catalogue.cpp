#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddBus(const domain::Bus& query) {
		buses.push_back(query);
		busname_to_bus[buses.back().bus_name] = &buses.back();

		for (auto stop : buses.back().route) {
			stopname_to_bus[stop->stop_name].insert(buses.back().bus_name);
		}
	}

	void TransportCatalogue::AddStop(const domain::Stop& query) {
		stops.push_back(query);
		stopname_to_stop[stops.back().stop_name] = &stops.back();
	}

	void TransportCatalogue::AddDistance(const std::string_view name_stop_one,
		const std::string_view name_stop_two, const uint64_t distance) {
			stops_distance[{FindStop(name_stop_one), FindStop(name_stop_two)}] = distance;
	}

	domain::Bus* TransportCatalogue::FindBus(std::string_view name_bus) const {
		if (busname_to_bus.count(name_bus)) {
			return busname_to_bus.at(name_bus);
		}
		return nullptr;
	}

	domain::Stop* TransportCatalogue::FindStop(std::string_view name_stop) const {
		if (stopname_to_stop.count(name_stop)) {
			return stopname_to_stop.at(name_stop);
		}
		return nullptr;
	}

	const std::set<std::string_view>& TransportCatalogue::GetBuses(std::string_view name_stop) const {
		static std::set<std::string_view> result;
		if (stopname_to_bus.count(name_stop)) {
			return stopname_to_bus.at(name_stop);
		}
		return result;
	}

	uint64_t TransportCatalogue::GetDistance(std::string_view stop_one, std::string_view stop_two) const {
		if (stops_distance.count({ FindStop(stop_one), FindStop(stop_two) })) {
			return stops_distance.at({ FindStop(stop_one), FindStop(stop_two) });
		}
		return stops_distance.at({ FindStop(stop_two), FindStop(stop_one) });
	}

	domain::QueryResultBus TransportCatalogue::GetInfoBus(std::string_view name_bus) const {
		domain::QueryResultBus result;
		std::unordered_set<domain::Stop*> count;
		double route_length_geo = 0;
		result.query_name = name_bus;
		auto bus = FindBus(result.query_name);
		if (bus != nullptr) {
			result.count_stops = static_cast<int>(bus->route.size());

			count.insert(bus->route.begin(), bus->route.end());
			result.unique_count_stops = static_cast<int>(count.size());

			for (size_t i = 0; i < bus->route.size() - 1; ++i) {
				result.route_length += GetDistance(bus->route[i]->stop_name, bus->route[i + 1]->stop_name);
				route_length_geo += geo::ComputeDistance(bus->route[i]->coordinates_,
					bus->route[i + 1]->coordinates_);
			}
			result.curvature = result.route_length / route_length_geo;
		}
		return result;
	}

	domain::QueryResultStop TransportCatalogue::GetInfoStop(std::string_view name_stop) const {
		domain::QueryResultStop result;
		result.query_name = name_stop;
		if (!GetBuses(result.query_name).empty()) {
			result.buses.insert(GetBuses(result.query_name).begin(), GetBuses(result.query_name).end());
		}
		return result;
	}

	const std::unordered_map<std::string_view, domain::Bus*>& TransportCatalogue::GetBusNameToBus() const {
		return busname_to_bus;
	}
}//close transport_catalogue