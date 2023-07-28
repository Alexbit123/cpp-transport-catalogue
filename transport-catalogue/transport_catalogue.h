#pragma once

#include "domain.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>

namespace transport_catalogue {
	using namespace std::literals;

	class TransportCatalogue {
	public:
		void AddBus(const domain::Bus& query);

		void AddStop(domain::Stop& query);

		void AddDistance(const std::string_view name_stop_one, 
			const std::string_view name_stop_two, const uint64_t distance);

		domain::Bus* FindBus(std::string_view name_bus) const;

		domain::Stop* FindStop(std::string_view name_stop) const;

		const std::set<std::string_view>& GetBuses(std::string_view name_stop) const;

		uint64_t GetDistance(std::string_view stop_one, std::string_view stop_two) const;
		int GetDistance(const domain::Stop* lhs, const domain::Stop* rhs) const;

		size_t GetVertexCount() const { return vertex_count_; }

		domain::QueryResultBus GetInfoBus(std::string_view name_bus) const;

		domain::QueryResultStop GetInfoStop(std::string_view name_stop) const;

		const std::unordered_map<std::string_view, domain::Bus*>& GetBusNameToBus() const;

		const std::deque<domain::Bus>& GetBusDeque() const;

		const std::deque<domain::Stop>& GetStopDeque() const;

		const std::unordered_map<std::pair<domain::Stop*, domain::Stop*>,
			uint64_t, domain::StopHasher>& GetStopsDistance() const;

	private:
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus;
		std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop;
		std::unordered_map<std::string_view, std::set<std::string_view>> stopname_to_bus;
		std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, 
			uint64_t, domain::StopHasher> stops_distance;
		std::deque<domain::Bus> buses;
		std::deque<domain::Stop> stops;
		size_t vertex_count_ = 0;
	};
}//close transport_catalogue
