#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"

class RequestHandler {
public:
	RequestHandler(const transport_catalogue::TransportCatalogue& db,
		const renderer::MapRenderer& renderer, router::TransportRouter& route);

	// Возвращает информацию о маршруте (запрос Bus)
	std::optional<domain::QueryResultBus> GetBusStat(const std::string_view& bus_name) const;

	// Возвращает маршруты, проходящие через
	const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;

	const std::unordered_map<std::string_view, domain::Bus*>& GetInfoBuses() const;

	svg::Document RenderMap() const;

	std::optional<router::CompletedRoute> CreateRoute(graph::VertexId from, graph::VertexId to);

private:
	// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
	const transport_catalogue::TransportCatalogue& db_;
	const renderer::MapRenderer& renderer_;
	router::TransportRouter& route_;
};