#include "parlay/sequence.h"
#include <cpam/cpam.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <jemalloc/jemalloc.h>
#include <limits>
#include <parlay/primitives.h>
#include <sys/types.h>
#include <utility>
template <typename V = uint32_t, typename T = uint64_t> struct interval_graph {
  using vertex_t = V;
  using timestamp_t = T;
  // set for each vertex
  struct nghs_entry {
    using key_t = vertex_t;
    static inline bool comp(key_t a, key_t b) { return a < b; }
  };
  using nghs_set = cpam::pam_set<nghs_entry>;
  // map for each vertex and its nghs
  struct vertex_entry {
    using key_t = vertex_t;
    using val_t = nghs_set;
    static inline bool comp(key_t a, key_t b) { return a < b; }
  };
  using vertex_map = cpam::pam_map<vertex_entry>;

  using instance_t = parlay::sequence<parlay::sequence<vertex_t>>;
  // (u,v,st,ed) edge (u,v) at time interval (st,ed)
  struct interval_edge_t {
    vertex_t u;
    vertex_t v;
    timestamp_t st;
    timestamp_t ed;
  };
  /* ins/del at time t grouped by vertex
                                (v1,u11)...(v1,u1k)
                               /
                      insertion ...
                     /         \
                    /           (vn,un1)...(vn,un)
  updates_at_time_t
                    \        (v1,u11)...(v1,u1k)
                    \       /
                    deletion ...
                            \
                            (vn,un1)...(vn,un)
  */
  using updates_t = parlay::sequence<std::pair<
      timestamp_t,
      std::pair<parlay::sequence<std::pair<
                    vertex_t, parlay::sequence<vertex_t>>>, // insertion
                parlay::sequence<std::pair<
                    vertex_t, parlay::sequence<vertex_t>>>>>>; // deletion

  vertex_t n;
  timestamp_t m;
  parlay::sequence<interval_edge_t> edges;
  parlay::sequence<std::pair<timestamp_t, vertex_map>> TG;

  interval_graph()
      : n(0), m(std::numeric_limits<timestamp_t>().max()),
        edges(parlay::sequence<interval_edge_t>(0)),
        TG(parlay::sequence<std::pair<timestamp_t, vertex_map>>(0)) {}
  interval_graph(const std::string &filename) {
    // std::ifstream ifs(filename);
    // if (!ifs.is_open()) {
    //   std::cerr << "Error: Cannot open file " << filename << '\n';
    //   std::abort();
    // }
    // ifs.read(reinterpret_cast<char *>(&n), sizeof(vertex_t));
    // ifs.read(reinterpret_cast<char *>(&m), sizeof(timestamp_t));
    // std::cout << n << " " << m << std::endl;
    // edges.resize(m);
    // ifs.read(reinterpret_cast<char *>(edges.begin()),
    //          sizeof(interval_edge_t) * m);
    create_index();
  }
  ~interval_graph() {}

public:
  void create_index() { update_index(edges); }
  instance_t retrive_graph_at(timestamp_t t) {}
  void update_index(parlay::sequence<interval_edge_t> &E) {
    updates_t updates = get_update_from_ie(E);
    vertex_map latest;
    if (!TG.empty())
      latest = TG.end()->second;
    std::cout << updates.size() << std::endl;
    for (auto it : updates) {
      auto timestamp = it.first;
      auto ins = it.second.first;
      auto del = it.second.second;
      std::cout << timestamp << " " << ins.size() << " " << del.size()
                << std::endl;
      for (auto ins_it : ins) {
        if (!latest.find(ins_it.first)) {
          nghs_set nghs(ins_it.second);
          std::cout << ins_it.first << " " << nghs.size() << std::endl;
          latest = vertex_map::insert(std::move(latest),
                                      std::pair(ins_it.first, nghs));
        }
      }
    }
  }

private:
  updates_t get_update_from_ie(parlay::sequence<interval_edge_t> &E) {
    updates_t updates;
    parlay::sequence<vertex_t> v1_at_1 = {2, 3, 4, 5};
    parlay::sequence<vertex_t> v2_at_1 = {1, 3, 4, 5};
    parlay::sequence<vertex_t> v3_at_1 = {1, 2, 4, 5};
    parlay::sequence<vertex_t> v4_at_1 = {1, 2, 3, 5};
    parlay::sequence<vertex_t> v5_at_1 = {1, 2, 3, 4};
    parlay::sequence<std::pair<vertex_t, parlay::sequence<vertex_t>>>
        insertion_at_1;
    insertion_at_1.push_back(std::pair(1, v1_at_1));
    insertion_at_1.push_back(std::pair(2, v2_at_1));
    insertion_at_1.push_back(std::pair(3, v3_at_1));
    insertion_at_1.push_back(std::pair(4, v4_at_1));
    insertion_at_1.push_back(std::pair(5, v5_at_1));
    parlay::sequence<std::pair<vertex_t, parlay::sequence<vertex_t>>>
        deletion_at_1;
    updates.push_back(std::pair(1, std::pair(insertion_at_1, deletion_at_1)));
    parlay::sequence<std::pair<vertex_t, parlay::sequence<vertex_t>>>
        insertion_at_2;
    parlay::sequence<vertex_t> v1_at_2 = {2, 3, 4, 5};
    parlay::sequence<vertex_t> v2_at_2 = {1};
    parlay::sequence<vertex_t> v3_at_2 = {1};
    parlay::sequence<vertex_t> v4_at_2 = {1};
    parlay::sequence<vertex_t> v5_at_2 = {1};
    parlay::sequence<std::pair<vertex_t, parlay::sequence<vertex_t>>>
        deletion_at_2 = {std::pair(1, v1_at_2), std::pair(2, v2_at_2),
                         std::pair(3, v3_at_2), std::pair(4, v4_at_2),
                         std::pair(5, v5_at_2)};
    updates.push_back(std::pair(2, std::pair(insertion_at_2, deletion_at_2)));
    return updates;
  }
};
