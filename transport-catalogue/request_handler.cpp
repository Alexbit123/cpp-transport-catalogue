#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, 
	const renderer::MapRenderer& renderer, router::TransportRouter& route)
	:db_(db), renderer_(renderer), route_(route) {
}

std::optional<domain::QueryResultBus> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return std::optional<domain::QueryResultBus>(db_.GetInfoBus(bus_name));
}

const std::set<std::string_view>*
RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	return &db_.GetBuses(stop_name);
}

const std::unordered_map<std::string_view, domain::Bus*>& RequestHandler::GetInfoBuses() const {
	return db_.GetBusNameToBus();
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.GetMap(GetInfoBuses());
}

std::optional<router::CompletedRoute> RequestHandler::CreateRoute(graph::VertexId from, graph::VertexId to) {
	return route_.ResultRoute(GetInfoBuses(), db_, from, to);
}
