#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#   ######      (!) 2022 by Giovanni Squillero <squillero@polito.it>
#  #######      +-------------------------------------------------------+
#  ####   \     |  *<*>*  THIS FILE IS NOT INTENDED FOR SHARING  *<*>*  |
#   ###G  c\    +-------------------------------------------------------+
#   ##     _\   The intellectual and technical concepts are proprietary.
#   |    _/     Dissemination or reproduction is forbidden and may result
#   |   _/      in civil charges and criminal penalties.

import logging
from pprint import pprint
import argparse
import csv
from math import sqrt
from os.path import isdir
import statistics


class Point:

    def __init__(self, name, side, x, y) -> None:
        if side == 'U':
            logging.warning(f"Converting U to T in point \"{name}\"")
            side = 'T'
        elif side == 'N':
            logging.warning(f"Converting N to B in point \"{name}\"")
            side = 'B'
        self.name = name
        assert side == 'T' or side == 'B', f"Side? {side}"
        self.side = side
        self.x = x
        self.y = y
        self.net = None

    def __str__(self):
        return f"{self.name}/{self.side}"


class Net:

    def __init__(self, name, num_points, *points) -> None:
        self.name = name
        self.points = set(points)
        assert len(self.points) == num_points

    def __str__(self):
        return f"{self.name} -> {self.points}"


class TestBlock:

    def __init__(self, name, points) -> None:
        self.name = name
        self.points = set(points)
        self.detected = False

    def __str__(self):
        return f"{self.name}: {self.points}"


def read_board(filename):
    points = dict()
    nets = dict()
    with open(filename) as file:
        for line in file:
            cmd, *tokens = line.split()
            if cmd == '.component':
                pass
            elif cmd == '.point':
                assert tokens[0] not in points
                points[tokens[0]] = Point(tokens[0], tokens[1], float(tokens[2]), float(tokens[3]))
            elif cmd == '.net':
                assert tokens[0] not in nets
                nets[tokens[0]] = Net(tokens[0], int(tokens[1]), *tokens[2:])
            else:
                assert False, line
    # check p<->n
    for p in points.values():
        for n in nets.values():
            if p.name in n.points:
                assert not p.net, 'Point in multiple Nets'
                p.net = n.name
        assert p.net, 'Point with no Net'
    logging.warning(f"Board contains {len(points):,} points and {len(nets):,} nets")
    return points, nets


def read_movements(filename):
    movements = list()
    with open(filename, newline='') as file:
        for row in csv.reader(file, delimiter=';'):
            movements.append(row[1:])
    return movements[1:]


def read_tests(filename, p):
    tests = dict()
    with open(filename) as file:
        for line in file:
            tag, name, num, *test_points = line.split()
            assert tag == '.test'
            assert len(test_points) == int(num)
            tests[name] = TestBlock(name, [p[n].net for n in test_points])
    return tests


def dropping(tests, nets):
    logging.debug(f"Nets: {nets}")
    for tb in tests:
        if tb.points <= nets:
            if not tb.detected:
                logging.debug(f"Covering {tb.__dict__}")
            tb.detected = True


def check_points(points):
    assert all(p1.x <= p2.x for p1, p2 in zip(points, points[1:4])), [f"{p}@x={p.x}" for p in points]
    assert all(p1.x <= p2.x for p1, p2 in zip(points[5:], points[6:])), [f"{p}@x={p.x}" for p in points]
    assert all(p.side == 'T' for p in points[:4]) and all(p.side == 'B' for p in points[4:]), [
        str(p) for p in points
    ]


def check_tplist(points, filename):
    with open(filename, newline='') as file:
        for row in csv.reader(file, delimiter=';'):
            logging.debug(f"{row}")


def main(board_filename, tests_filename, movements_filename, tplist_filename):
    try:
        points, nets = read_board(board_filename)
        tests = read_tests(tests_filename, points)
        movements = read_movements(movements_filename)
        if tplist_filename:
            check_tplist(points, tplist_filename)
    except OSError as problem:
        logging.error(f"Yeuch: {problem}")
        exit(1)

    active_tests = list(tests.values())
    tot_dist = list()
    current_pos = [points[n] for n in movements[0]]
    check_points(current_pos)
    visited = set()
    line = 0
    for step in movements[1:] + [None]:
        line += 1
        # detection
        nets = frozenset(p.net for p in current_pos)
        assert nets not in visited, f'Line {line}: State dup: {nets}'
        visited.add(nets)
        dropping(active_tests, nets)

        if step:
            next_pos = [points[n] for n in step]
            check_points(next_pos)
            d = max(sqrt((p1.x - p2.x)**2 + (p1.y - p2.y)**2) for p1, p2 in zip(current_pos, next_pos))
            tot_dist.append(d)
            current_pos = next_pos

    logging.info(f"path: {len(movements):,} movements; distance tot: {sum(tot_dist)}")
    logging.info(f"max: {max(tot_dist):.2f}; min: {min(tot_dist):.2f}")
    logging.info(
        f"mean: {statistics.mean(tot_dist):.2f}; stdev: {statistics.stdev(tot_dist):.2f}; median: {statistics.median(tot_dist):.2f}; hmean: {statistics.harmonic_mean(tot_dist):.2f}"
    )
    if any(not tb.detected for tb in active_tests):
        logging.warning(f"Found undetected test blocks")
        for testblock in [tb for tb in active_tests if not tb.detected]:
            logging.warning(f"* {testblock.name}: {testblock.points}")


if __name__ == "__main__":
    logging.basicConfig(format="%(asctime)s %(levelname)s:: %(message)s", datefmt="%H:%M:%S")

    parser = argparse.ArgumentParser()
    parser.add_argument('board', help="Board description")
    parser.add_argument('tests', help="Test list")
    parser.add_argument('movements', help="Movement list (CSV)")
    parser.add_argument('-t', '--tplist', dest='tplist', default=None, help="SPEA's tplist (CSV)")

    parser.add_argument('-v', '--verbose', action='count', default=1, help="increase log verbosity")
    parser.add_argument('-d',
                        '--debug',
                        action='store_const',
                        dest='verbose',
                        const=2,
                        help="log debug messages (same as -vv)")
    args = parser.parse_args()

    if args.verbose == 0:
        logging.getLogger().setLevel(level=logging.WARNING)
    elif args.verbose == 1:
        logging.getLogger().setLevel(level=logging.INFO)
    elif args.verbose == 2:
        logging.getLogger().setLevel(level=logging.DEBUG)

    # eg. python checker.py board\board.txt board\test.txt board\tracePoints.csv
    main(args.board, args.tests, args.movements, args.tplist)
