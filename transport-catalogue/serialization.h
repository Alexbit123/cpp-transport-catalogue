#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"

#include <filesystem>
#include <fstream>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <transport_router.pb.h>
#include <graph.pb.h>


namespace proto_info {
	class ProtoInfo {
	public:
		ProtoInfo() = default;
		ProtoInfo(transport_catalogue::TransportCatalogue& db,
			renderer::MapRenderer& renderer, router::TransportRouter& route);

		void Serialization(const std::filesystem::path& path);
		void Deserialization(const std::filesystem::path& path);

		std::vector<domain::Stop> ParseProtoStops();
		std::vector<domain::Distance> ParseProtoDistance();
		std::vector<domain::Bus> ParseProtoBuses(transport_catalogue::TransportCatalogue& db);
		void ParseProtoMap(renderer::MapRenderer& renderer);
		void ParseProtoTransportRouter(router::TransportRouter& route, transport_catalogue::TransportCatalogue& db);
	private:
		t_catalogue_proto::TransportCatalogue t_catalogue_;

		void AddBuses(transport_catalogue::TransportCatalogue& db);
		void AddDistance(transport_catalogue::TransportCatalogue& db);
		void AddStops(transport_catalogue::TransportCatalogue& db);
		void AddMap(renderer::MapRenderer& renderer);
		void AddRoute(router::TransportRouter& route);
		void AddColorInProto(renderer::MapRenderer& renderer);
		void AddColorPaletteInProto(renderer::MapRenderer& renderer);
		void AddColorOutProto(renderer::MapRenderer& renderer);
		void AddColorPaletteOutProto(renderer::MapRenderer& renderer);
		void HelperAddGraphInProto(router::TransportRouter& route);
		void HelperAddEdgesInProto(router::TransportRouter& route);
		void HelperAddRouterInProto(router::TransportRouter& route);
		void ParseProtoGraph(router::TransportRouter& route);
		void ParseProtoEdgeInfo(router::TransportRouter& route, transport_catalogue::TransportCatalogue& db);
		void ParseProtoRouter(router::TransportRouter& route);
	};
}

