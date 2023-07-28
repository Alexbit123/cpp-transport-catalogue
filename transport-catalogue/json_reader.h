#pragma once

#include "request_handler.h"
#include "json.h"
#include "json_builder.h"
//#include <filesystem>

namespace json_reader {
	void Read(transport_catalogue::TransportCatalogue& db, 
		renderer::MapRenderer& renderer, router::TransportRouter& route, std::istream& in, std::string& query);

	void Out(transport_catalogue::TransportCatalogue& db,
		renderer::MapRenderer& renderer, router::TransportRouter& route, std::istream& in,
		std::ostream& out, std::string& query);

	namespace out_json_query {
		void Output(transport_catalogue::TransportCatalogue& db, 
			std::string& query, std::istream& in ,std::ostream& out, 
			renderer::MapRenderer& renderer, router::TransportRouter& route);
	}

	namespace add_json_query {
		void AddBaseRequests(transport_catalogue::TransportCatalogue& db,
			renderer::MapRenderer& renderer, router::TransportRouter& route, std::string& query);

	}//close add_json_query
	namespace detail {
		json::Dict GetInfoBus(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem);

		json::Dict GetInfoStop(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem);

		json::Dict GetInfoMap(transport_catalogue::TransportCatalogue& db,
			const json::Dict& elem, renderer::MapRenderer& renderer, router::TransportRouter& route);

		void PrintResult(transport_catalogue::TransportCatalogue& db, 
			const json::Node& elem, std::ostream& out, renderer::MapRenderer& renderer, router::TransportRouter& route);

		std::vector<domain::Stop> ParseQueryStop(const json::Node& elem);

		std::vector<domain::Distance> ParseQueryDistance(const json::Node& elem);

		std::vector<domain::Bus> ParseQueryBus(const json::Node& elem,
			transport_catalogue::TransportCatalogue& db);

		renderer::MapRenderer ParseQueryMap(const json::Node& elem);

		router::TransportRouter ParseQueryRoute(const json::Node& elem);

		svg::Color ParseColor(const json::Node& node_result);

		std::filesystem::path ParseQuerySerialization(const json::Node& elem);
	}//close detail
}//close json_reader