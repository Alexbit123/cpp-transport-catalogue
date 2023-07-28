#include "transport_router.h"

namespace router {
	std::optional<CompletedRoute> TransportRouter::ComputeRoute(graph::VertexId from, graph::VertexId to) {
		std::optional<graph::Router<double>::RouteInfo> build_route_ = router_->BuildRoute(from, to);
		if (!build_route_) {
			return std::nullopt;
		}

		if (build_route_->weight < eps) {
			return CompletedRoute({ 0, {} });
		}

		CompletedRoute result;
		result.total_time = build_route_->weight;
		result.route.reserve(build_route_->edges.size());

		for (auto& edge : build_route_->edges)
		{
			EdgeInfo& info = edges_.at(edge);
			double wait_time_ = static_cast<double>(wait);
			double run_time_ = graph_.GetEdge(edge).weight - wait;
			result.route.push_back(CompletedRoute::Line{ info.stop, info.bus, wait_time_, run_time_ ,info.count });

		}
		return result;
	}

	void TransportRouter::CreateGraph(const std::unordered_map <std::string_view,
		domain::Bus*>& info_bus, const transport_catalogue::TransportCatalogue& db) {
		if (graph_.GetVertexCount() > 0) {
			return;
		}

		graph_.SetVertexCount(db.GetVertexCount());
		double bus_velocity = speed * m_to_mmin;

		for (auto [bus_name, info] : info_bus) {
			const domain::Bus* bus = info;
			auto it = bus->route.begin();
			if (it == bus->route.end() || it + 1 == bus->route.end()) {
				continue;
			}
			for (; it + 1 != bus->route.end(); ++it) {
				double time = static_cast<double>(wait);

				for (auto next_vertex = it + 1; next_vertex != bus->route.end(); ++next_vertex) {
					time += db.GetDistance(*prev(next_vertex), *next_vertex) / bus_velocity;
					edges_[graph_.AddEdge({ (*it)->vertex_id,(*next_vertex)->vertex_id, time })]
						= { *it,bus,static_cast<int>(next_vertex - it) };
				}
			}
		}
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

	std::optional<CompletedRoute> TransportRouter::ResultRoute(const std::unordered_map<std::string_view,
		domain::Bus*>& info_bus, const transport_catalogue::TransportCatalogue& db,
		graph::VertexId from, graph::VertexId to) {
		CreateGraph(info_bus, db);
		return ComputeRoute(from, to);
	}

	graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() {
		return graph_;
	}

	std::unordered_map<graph::EdgeId, EdgeInfo>& TransportRouter::GetEdges() {
		return edges_;
	}

	std::unique_ptr<graph::Router<double>>& TransportRouter::GetRouter() {
		return router_;
	}
}
