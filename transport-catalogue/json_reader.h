#pragma once

#include "request_handler.h"
#include "json.h"
#include "json_builder.h"

namespace json_reader {
	void Read(transport_catalogue::TransportCatalogue& db, 
		renderer::MapRenderer& renderer, std::istream& in, std::ostream& out, bool need_out_stat_request = false);

	namespace out_json_query {
		void Output(transport_catalogue::TransportCatalogue& db, 
			std::string& query, std::ostream& out, renderer::MapRenderer& renderer);
	}

	namespace add_json_query {
		void AddBaseRequests(transport_catalogue::TransportCatalogue& db,
			renderer::MapRenderer& renderer, std::string& query);

	}//close add_json_query
	namespace detail {
		json::Dict GetInfoBus(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem);

		json::Dict GetInfoStop(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem);

		json::Dict GetInfoMap(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem, renderer::MapRenderer& renderer);

		void PrintResult(transport_catalogue::TransportCatalogue& db, 
			const json::Node& elem, std::ostream& out, renderer::MapRenderer& renderer);

		std::vector<domain::Stop> ParseQueryStop(const json::Node& elem);

		std::vector<domain::Distance> ParseQueryDistance(const json::Node& elem);

		std::vector<domain::Bus> ParseQueryBus(const json::Node& elem,
			transport_catalogue::TransportCatalogue& db);

		renderer::MapRenderer ParseQueryMap(const json::Node& elem);

		svg::Color ParseColor(const json::Node& node_result);
	}//close detail
}//close json_reader