#pragma once
#include "graph.h"
#include "ranges.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <map>

const double eps = 1e-6;
const double m_to_mmin = 1000 * 1.0 / 60;

namespace router {
	using namespace std::string_literals;

	struct EdgeInfo
	{
		const domain::Stop* stop;
		const domain::Bus* bus;
		int count;
	};

	struct CompletedRoute {
		struct Line {
			const domain::Stop* stop;
			const domain::Bus* bus;
			double wait_time;
			double run_time;
			int count_stops;
		};
		double total_time;
		std::vector<Line> route;
	};

	class TransportRouter {
	private:
		graph::DirectedWeightedGraph<double> graph_;
		std::unordered_map<graph::EdgeId, EdgeInfo> edges_;
		std::unique_ptr<graph::Router<double>> router_;

		std::optional<CompletedRoute> ComputeRoute(graph::VertexId from, graph::VertexId to);

		void CreateGraph(const std::unordered_map <std::string_view,
			domain::Bus*>& info_bus, const transport_catalogue::TransportCatalogue& db);
	public:
		std::optional<CompletedRoute> ResultRoute(const std::unordered_map<std::string_view,
			domain::Bus*>& info_bus, const transport_catalogue::TransportCatalogue& db,
			graph::VertexId from, graph::VertexId to);
		int wait = 0;
		double speed = 0.0;
	};
} //router