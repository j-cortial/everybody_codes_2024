#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <istream>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;
using Herb = char;

struct Forest {  // NOLINT(cppcoreguidelines-pro-type-member-init)
  std::set<Loc> open_tiles;
  std::map<Loc, Herb> herbs;
  Loc start;
};

auto parse_input(std::istream&& in) {
  Forest result;

  auto lines = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n');

  for (const auto [j, row] : std::views::enumerate(lines)) {
    for (const auto [i, c] : std::views::enumerate(row)) {
      const Loc loc{Coord(i), Coord(j)};
      if (c == '.') {
        result.open_tiles.insert(loc);
        if (loc[1] == Coord{}) {
          result.start = loc;
        }
      } else if (std::isalpha(c) != 0) {
        result.open_tiles.insert(loc);
        result.herbs.emplace(loc, c);
      }
    }
  }

  return result;
}

auto operator+(const Loc& a, const Loc& b) -> Loc { return {a[0] + b[0], a[1] + b[1]}; }

constexpr std::array<Loc, 4> moves{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

using PathLength = std::uint64_t;

auto shortest_path_lengths(const std::set<Loc>& open_tiles, const Loc start,
                           const std::map<Loc, Herb>& targets) -> std::map<Loc, PathLength> {
  std::map<Loc, PathLength> result;

  std::set<Loc> explored{start};
  std::vector<Loc> front{start};

  PathLength path_length{1};

  while (!front.empty()) {
    std::vector<Loc> next_front;

    for (const Loc& current : front) {
      for (const Loc& move : moves) {
        const Loc next = current + move;
        if (open_tiles.contains(next) && explored.insert(next).second) {
          if (const auto it = targets.find(next); it != targets.end()) {
            result.emplace(next, path_length);
          }
          next_front.push_back(next);
        }
      }
    }

    if (result.size() == targets.size()) {
      break;
    }

    ++path_length;
    front = std::move(next_front);
  }

  return result;
}

using Edges = std::multimap<Loc, std::pair<Loc, PathLength>>;

void fill_edges(Edges& edges, const Loc origin, const std::map<Loc, PathLength>& ends) {
  for (const auto& end : ends) {
    edges.emplace(origin, end);
  }
}

void fill_reciprocal_edges(Edges& edges, const Loc origin, const std::map<Loc, PathLength>& ends) {
  for (const auto& end : ends) {
    edges.emplace(end.first, std::make_pair(origin, end.second));
  }
}

auto compressed_paths(const Forest& forest) -> Edges {
  Edges result;

  fill_edges(result, forest.start,
             shortest_path_lengths(forest.open_tiles, forest.start, forest.herbs));

  const auto all_herbs = std::views::values(forest.herbs) | std::ranges::to<std::set>();
  for (const auto herb : all_herbs) {
    const auto starts =
        std::views::filter(forest.herbs, [&](const auto& pair) { return pair.second == herb; }) |
        std::views::keys | std::ranges::to<std::vector>();
    const auto targets =
        std::views::filter(forest.herbs, [&](const auto& pair) { return pair.second > herb; }) |
        std::ranges::to<std::map>();
    for (const Loc& start : starts) {
      const auto path_lengths = shortest_path_lengths(forest.open_tiles, start, targets);
      fill_edges(result, start, path_lengths);
      fill_reciprocal_edges(result, start, path_lengths);
    }
  }

  return result;
}

auto herb_type_count(const Forest& forest) -> std::size_t {
  const auto all_herbs = std::views::values(forest.herbs) | std::ranges::to<std::set>();
  return all_herbs.size();
}

using HerbSet = std::uint32_t;

auto herb_code(const Herb herb) -> HerbSet { return 1U << (herb - 'A'); }

auto contains(const HerbSet& herb_set, const Herb herb) {
  return (herb_set & herb_code(herb)) != 0U;
}

auto insert(const HerbSet& herb_set, const Herb herb) { return herb_set | herb_code(herb); }

struct Status {  // NOLINT(cppcoreguidelines-pro-type-member-init)
  Loc loc;
  HerbSet collected_herbs{};

  constexpr auto operator<=>(const Status&) const noexcept = default;
};

auto solve(const Forest& forest) {
  const Edges edges = compressed_paths(forest);

  std::map<Status, PathLength> front{{{.loc = forest.start, .collected_herbs = {}}, 0}};
  for (auto _ : std::views::iota(0UZ, herb_type_count(forest))) {
    std::map<Status, PathLength> new_front;

    for (const auto& [current_status, current_length] : front) {
      const auto& [current_loc, current_herbs] = current_status;
      const auto candidate_range = edges.equal_range(current_loc);
      for (const auto& [candidate_loc, candidate_length_delta] : std::views::values(
               std::ranges::subrange(candidate_range.first, candidate_range.second))) {
        const Herb new_herb = forest.herbs.find(candidate_loc)->second;
        if (!contains(current_herbs, new_herb)) {
          const auto next_herbs = insert(current_herbs, new_herb);
          const auto next_length = current_length + candidate_length_delta;
          const auto [it, inserted] = new_front.emplace(
              Status{.loc = candidate_loc, .collected_herbs = next_herbs}, next_length);
          if (!inserted) {
            it->second = std::min(it->second, next_length);
          }
        }
      }
    }

    front = std::move(new_front);
  }

  return std::ranges::min(std::views::transform(front, [&](const auto& entry) {
    const auto& [status, path_length] = entry;
    const auto loc = status.loc;
    const auto candidate_edges = edges.equal_range(forest.start);
    const auto range = std::ranges::subrange(candidate_edges.first, candidate_edges.second);
    const auto it =
        std::ranges::find_if(range, [&](const auto& entry) { return entry.second.first == loc; });
    return path_length + it->second.second;
  }));
}

auto solve_part1(const auto& input) { return solve(input); }

auto solve_part2(const auto& input) { return solve(input); }

auto solve_part3(const auto& input) { return solve(input); }

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
