// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "chromeos/obsolete_logging.h"
#include "update_engine/cycle_breaker.h"
#include "update_engine/graph_types.h"
#include "update_engine/utils.h"

using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::vector;

namespace chromeos_update_engine {

class CycleBreakerTest : public ::testing::Test {};

TEST(CycleBreakerTest, SimpleTest) {
  int counter = 0;
  const Vertex::Index n_a = counter++;
  const Vertex::Index n_b = counter++;
  const Vertex::Index n_c = counter++;
  const Vertex::Index n_d = counter++;
  const Vertex::Index n_e = counter++;
  const Vertex::Index n_f = counter++;
  const Vertex::Index n_g = counter++;
  const Vertex::Index n_h = counter++;
  const Graph::size_type kNodeCount = counter++;

  Graph graph(kNodeCount);

  graph[n_a].out_edges.insert(make_pair(n_e, EdgeProperties()));
  graph[n_a].out_edges.insert(make_pair(n_f, EdgeProperties()));
  graph[n_b].out_edges.insert(make_pair(n_a, EdgeProperties()));
  graph[n_c].out_edges.insert(make_pair(n_d, EdgeProperties()));
  graph[n_d].out_edges.insert(make_pair(n_e, EdgeProperties()));
  graph[n_d].out_edges.insert(make_pair(n_f, EdgeProperties()));
  graph[n_e].out_edges.insert(make_pair(n_b, EdgeProperties()));
  graph[n_e].out_edges.insert(make_pair(n_c, EdgeProperties()));
  graph[n_e].out_edges.insert(make_pair(n_f, EdgeProperties()));
  graph[n_f].out_edges.insert(make_pair(n_g, EdgeProperties()));
  graph[n_g].out_edges.insert(make_pair(n_h, EdgeProperties()));
  graph[n_h].out_edges.insert(make_pair(n_g, EdgeProperties()));

  CycleBreaker breaker;
  
  set<Edge> broken_edges;
  breaker.BreakCycles(graph, &broken_edges);
  
  // The following cycles must be cut:
  // A->E->B
  // C->D->E
  // G->H

  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_a, n_e)) ||
              utils::SetContainsKey(broken_edges, make_pair(n_e, n_b)) ||
              utils::SetContainsKey(broken_edges, make_pair(n_b, n_a)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_c, n_d)) ||
              utils::SetContainsKey(broken_edges, make_pair(n_d, n_e)) ||
              utils::SetContainsKey(broken_edges, make_pair(n_e, n_c)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_g, n_h)) ||
              utils::SetContainsKey(broken_edges, make_pair(n_h, n_g)));
  EXPECT_EQ(3, broken_edges.size());
}

namespace {
pair<Vertex::Index, EdgeProperties> EdgeWithWeight(Vertex::Index dest,
uint64_t weight) {
  EdgeProperties props;
  props.extents.resize(1);
  props.extents[0].set_num_blocks(weight);
  return make_pair(dest, props);
}
}  // namespace {}

TEST(CycleBreakerTest, WeightTest) {
  int counter = 0;
  const Vertex::Index n_a = counter++;
  const Vertex::Index n_b = counter++;
  const Vertex::Index n_c = counter++;
  const Vertex::Index n_d = counter++;
  const Vertex::Index n_e = counter++;
  const Vertex::Index n_f = counter++;
  const Vertex::Index n_g = counter++;
  const Vertex::Index n_h = counter++;
  const Vertex::Index n_i = counter++;
  const Vertex::Index n_j = counter++;
  const Graph::size_type kNodeCount = counter++;

  Graph graph(kNodeCount);

  graph[n_a].out_edges.insert(EdgeWithWeight(n_b, 4));
  graph[n_a].out_edges.insert(EdgeWithWeight(n_f, 3));
  graph[n_a].out_edges.insert(EdgeWithWeight(n_h, 2));
  graph[n_b].out_edges.insert(EdgeWithWeight(n_a, 3));
  graph[n_b].out_edges.insert(EdgeWithWeight(n_c, 4));
  graph[n_c].out_edges.insert(EdgeWithWeight(n_b, 5));
  graph[n_c].out_edges.insert(EdgeWithWeight(n_d, 3));
  graph[n_d].out_edges.insert(EdgeWithWeight(n_a, 6));
  graph[n_d].out_edges.insert(EdgeWithWeight(n_e, 3));
  graph[n_e].out_edges.insert(EdgeWithWeight(n_d, 4));
  graph[n_e].out_edges.insert(EdgeWithWeight(n_g, 5));
  graph[n_f].out_edges.insert(EdgeWithWeight(n_g, 2));
  graph[n_g].out_edges.insert(EdgeWithWeight(n_f, 3));
  graph[n_g].out_edges.insert(EdgeWithWeight(n_d, 5));
  graph[n_h].out_edges.insert(EdgeWithWeight(n_i, 8));
  graph[n_i].out_edges.insert(EdgeWithWeight(n_e, 4));
  graph[n_i].out_edges.insert(EdgeWithWeight(n_h, 9));
  graph[n_i].out_edges.insert(EdgeWithWeight(n_j, 6));

  CycleBreaker breaker;
  
  set<Edge> broken_edges;
  breaker.BreakCycles(graph, &broken_edges);

  // These are required to be broken:
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_b, n_a)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_b, n_c)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_d, n_e)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_f, n_g)));
  EXPECT_TRUE(utils::SetContainsKey(broken_edges, make_pair(n_h, n_i)));
}

}  // namespace chromeos_update_engine