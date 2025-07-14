#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <limits>
#include <map>
#include <print>
#include <queue>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;
using Level = std::uint8_t;

struct Maze {  // NOLINT(cppcoreguidelines-pro-type-member-init)
  std::map<Loc, Level> platforms;
  std::vector<Loc> starts;
  Loc exit;
};

auto parse_input(std::istream&& in) {
  Maze result;

  auto rows = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n');

  for (const auto& [j, row] : std::views::enumerate(rows)) {
    for (const auto& [i, c] : std::views::enumerate(row)) {
      const Loc loc{Coord(i), Coord(j)};
      if (std::isdigit(c) != 0) {
        result.platforms.insert({loc, c - '0'});
      } else if (c == 'S') {
        result.platforms.insert({loc, 0});
        result.starts.push_back(loc);
      } else if (c == 'E') {
        result.platforms.insert({loc, 0});
        result.exit = loc;
      }
    }
  }

  return result;
}

auto operator+(const Loc& a, const Loc& b) -> Loc { return {a[0] + b[0], a[1] + b[1]}; }

constexpr std::array<Loc, 4> moves{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

auto shortest_path(const Maze& maze) -> std::size_t {
  using Candidate = std::pair<std::size_t, Loc>;
  std::priority_queue<Candidate, std::vector<Candidate>, std::greater<>> queue;
  for (const Loc& start : maze.starts) {
    queue.emplace(0UZ, start);
  }

  std::set<Loc> explored;

  while (!queue.empty()) {
    const auto [length, loc] = queue.top();
    queue.pop();

    if (loc == maze.exit) {
      return length;
    }

    if (explored.insert(loc).second) {
      const auto current_level = maze.platforms.find(loc)->second;
      for (const Loc& move : moves) {
        const Loc next = loc + move;
        if (!explored.contains(next)) {
          const auto next_it = maze.platforms.find(next);
          if (next_it != maze.platforms.end()) {
            const auto next_level = next_it->second;
            const std::size_t delta =
                std::max(current_level, next_level) - std::min(current_level, next_level);
            const std::size_t distance = std::min(delta, 10UZ - delta);
            queue.emplace(length + distance + 1UZ, next);
          }
        }
      }
    }
  }

  return std::numeric_limits<std::size_t>::max();
}

auto solve_part1(const auto& input) { return shortest_path(input); }

auto solve_part2(const auto& input) { return shortest_path(input); }

auto solve_part3(const auto& input) { return shortest_path(input); }

auto main() -> int {
  const auto input1 = parse_input(std::ifstream{"input1.txt"});
  const auto answer1 = solve_part1(input1);
  std::println("The answer to part #1 is {}", answer1);

  const auto input2 = parse_input(std::ifstream{"input2.txt"});
  const auto answer2 = solve_part2(input2);
  std::println("The answer to part #2 is {}", answer2);

  const auto input3 = parse_input(std::ifstream{"input3.txt"});
  const auto answer3 = solve_part3(input3);
  std::println("The answer to part #3 is {}", answer3);
}
