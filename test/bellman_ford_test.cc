/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2009
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

#include <lemon/concepts/digraph.h>
#include <lemon/smart_graph.h>
#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/bellman_ford.h>
#include <lemon/path.h>

#include "graph_test.h"
#include "test_tools.h"

using namespace lemon;

char test_lgf[] =
  "@nodes\n"
  "label\n"
  "0\n"
  "1\n"
  "2\n"
  "3\n"
  "4\n"
  "@arcs\n"
  "    length\n"
  "0 1 3\n"
  "1 2 -3\n"
  "1 2 -5\n"
  "1 3 -2\n"
  "0 2 -1\n"
  "1 2 -4\n"
  "0 3 2\n"
  "4 2 -5\n"
  "2 3 1\n"
  "@attributes\n"
  "source 0\n"
  "target 3\n";


void checkBellmanFordCompile()
{
  typedef int Value;
  typedef concepts::Digraph Digraph;
  typedef concepts::ReadMap<Digraph::Arc,Value> LengthMap;
  typedef BellmanFord<Digraph, LengthMap> BF;
  typedef Digraph::Node Node;
  typedef Digraph::Arc Arc;

  Digraph gr;
  Node s, t, n;
  Arc e;
  Value l;
  int k;
  bool b;
  BF::DistMap d(gr);
  BF::PredMap p(gr);
  LengthMap length;
  concepts::Path<Digraph> pp;

  {
    BF bf_test(gr,length);
    const BF& const_bf_test = bf_test;

    bf_test.run(s);
    bf_test.run(s,k);

    bf_test.init();
    bf_test.addSource(s);
    bf_test.addSource(s, 1);
    b = bf_test.processNextRound();
    b = bf_test.processNextWeakRound();

    bf_test.start();
    bf_test.checkedStart();
    bf_test.limitedStart(k);

    l  = const_bf_test.dist(t);
    e  = const_bf_test.predArc(t);
    s  = const_bf_test.predNode(t);
    b  = const_bf_test.reached(t);
    d  = const_bf_test.distMap();
    p  = const_bf_test.predMap();
    pp = const_bf_test.path(t);
    
    for (BF::ActiveIt it(const_bf_test); it != INVALID; ++it) {}
  }
  {
    BF::SetPredMap<concepts::ReadWriteMap<Node,Arc> >
      ::SetDistMap<concepts::ReadWriteMap<Node,Value> >
      ::SetOperationTraits<BellmanFordDefaultOperationTraits<Value> >
      ::Create bf_test(gr,length);

    LengthMap length_map;
    concepts::ReadWriteMap<Node,Arc> pred_map;
    concepts::ReadWriteMap<Node,Value> dist_map;
    
    bf_test
      .lengthMap(length_map)
      .predMap(pred_map)
      .distMap(dist_map);

    bf_test.run(s);
    bf_test.run(s,k);

    bf_test.init();
    bf_test.addSource(s);
    bf_test.addSource(s, 1);
    b = bf_test.processNextRound();
    b = bf_test.processNextWeakRound();

    bf_test.start();
    bf_test.checkedStart();
    bf_test.limitedStart(k);

    l  = bf_test.dist(t);
    e  = bf_test.predArc(t);
    s  = bf_test.predNode(t);
    b  = bf_test.reached(t);
    pp = bf_test.path(t);
  }
}

void checkBellmanFordFunctionCompile()
{
  typedef int Value;
  typedef concepts::Digraph Digraph;
  typedef Digraph::Arc Arc;
  typedef Digraph::Node Node;
  typedef concepts::ReadMap<Digraph::Arc,Value> LengthMap;

  Digraph g;
  bool b;
  bellmanFord(g,LengthMap()).run(Node());
  b = bellmanFord(g,LengthMap()).run(Node(),Node());
  bellmanFord(g,LengthMap())
    .predMap(concepts::ReadWriteMap<Node,Arc>())
    .distMap(concepts::ReadWriteMap<Node,Value>())
    .run(Node());
  b=bellmanFord(g,LengthMap())
    .predMap(concepts::ReadWriteMap<Node,Arc>())
    .distMap(concepts::ReadWriteMap<Node,Value>())
    .path(concepts::Path<Digraph>())
    .dist(Value())
    .run(Node(),Node());
}


template <typename Digraph, typename Value>
void checkBellmanFord() {
  TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
  typedef typename Digraph::template ArcMap<Value> LengthMap;

  Digraph gr;
  Node s, t;
  LengthMap length(gr);

  std::istringstream input(test_lgf);
  digraphReader(gr, input).
    arcMap("length", length).
    node("source", s).
    node("target", t).
    run();

  BellmanFord<Digraph, LengthMap>
    bf(gr, length);
  bf.run(s);
  Path<Digraph> p = bf.path(t);

  check(bf.reached(t) && bf.dist(t) == -1, "Bellman-Ford found a wrong path.");
  check(p.length() == 3, "path() found a wrong path.");
  check(checkPath(gr, p), "path() found a wrong path.");
  check(pathSource(gr, p) == s, "path() found a wrong path.");
  check(pathTarget(gr, p) == t, "path() found a wrong path.");
  
  ListPath<Digraph> path;
  Value dist;
  bool reached = bellmanFord(gr,length).path(path).dist(dist).run(s,t);

  check(reached && dist == -1, "Bellman-Ford found a wrong path.");
  check(path.length() == 3, "path() found a wrong path.");
  check(checkPath(gr, path), "path() found a wrong path.");
  check(pathSource(gr, path) == s, "path() found a wrong path.");
  check(pathTarget(gr, path) == t, "path() found a wrong path.");

  for(ArcIt e(gr); e!=INVALID; ++e) {
    Node u=gr.source(e);
    Node v=gr.target(e);
    check(!bf.reached(u) || (bf.dist(v) - bf.dist(u) <= length[e]),
          "Wrong output. dist(target)-dist(source)-arc_length=" <<
          bf.dist(v) - bf.dist(u) - length[e]);
  }

  for(NodeIt v(gr); v!=INVALID; ++v) {
    if (bf.reached(v)) {
      check(v==s || bf.predArc(v)!=INVALID, "Wrong tree.");
      if (bf.predArc(v)!=INVALID ) {
        Arc e=bf.predArc(v);
        Node u=gr.source(e);
        check(u==bf.predNode(v),"Wrong tree.");
        check(bf.dist(v) - bf.dist(u) == length[e],
              "Wrong distance! Difference: " <<
              bf.dist(v) - bf.dist(u) - length[e]);
      }
    }
  }
}

int main() {
  checkBellmanFord<ListDigraph, int>();
  checkBellmanFord<SmartDigraph, double>();
  return 0;
}
