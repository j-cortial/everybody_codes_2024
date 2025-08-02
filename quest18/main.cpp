#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;

struct Farm {
  std::set<Loc> open_tiles;
  std::set<Loc> palm_trees;
  Loc bounds{};
};

auto parse_input(std::istream&& in) {
  auto tile_pairs =
      std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
      std::views::enumerate | std::views::transform([](auto&& row_pair) {
        auto&& [i, row] = row_pair;
        return std::views::enumerate(row) | std::views::transform([i](auto&& tile_pair) {
                 auto&& [j, tile] = tile_pair;
                 return std::make_pair(Loc{Coord(i), Coord(j)}, tile);
               });
      }) |
      std::views::join;

  Farm result;

  for (const auto& [loc, tile] : tile_pairs) {
    result.bounds[0] = std::max(result.bounds[0], loc[0]);
    result.bounds[1] = std::max(result.bounds[1], loc[1]);

    if (tile == 'P') {
      result.open_tiles.insert(loc);
      result.palm_trees.insert(loc);
    } else if (tile == '.') {
      result.open_tiles.insert(loc);
    }
  }

  return result;
}

auto starting_locations(const Farm& farm) -> std::vector<Loc> {
  return std::views::filter(farm.open_tiles,
                            [&](const Loc& loc) {
                              return loc[0] == Coord{} || loc[0] == farm.bounds[0] ||
                                     loc[1] == Coord{0} || loc[1] == farm.bounds[1];
                            }) |
         std::ranges::to<std::vector>();
}

auto operator+(const Loc& a, const Loc& b) noexcept -> Loc { return {a[0] + b[0], a[1] + b[1]}; }

constexpr std::array<Loc, 4> moves{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

auto furthest_tree(const Farm& farm) {
  Coord result{};

  std::vector<Loc> front = starting_locations(farm);
  std::set<Loc> explored{front.begin(), front.end()};
  std::size_t tree_count{};

  while (tree_count < farm.palm_trees.size()) {
    std::vector<Loc> next_front;

    for (const Loc& loc : front) {
      for (const Loc& move : moves) {
        const Loc candidate = loc + move;
        if (farm.open_tiles.contains(candidate) && explored.insert(candidate).second) {
          next_front.push_back(candidate);
          if (farm.palm_trees.contains(candidate)) {
            ++tree_count;
          }
        }
      }
    }

    front = std::move(next_front);
    ++result;
  }

  return result;
}

auto solve_part1(const auto& input) { return furthest_tree(input); }

auto solve_part2(const auto& input) { return furthest_tree(input); }

auto distances(const Farm& farm, const Loc& start) -> std::vector<Coord> {
  std::vector<Coord> result(farm.open_tiles.size());

  std::vector<Loc> front{start};
  std::set<Loc> explored{front.begin(), front.end()};
  Coord distance{};

  while (explored.size() < farm.open_tiles.size()) {
    std::vector<Loc> next_front;
    ++distance;

    for (const Loc& loc : front) {
      for (const Loc& move : moves) {
        const Loc candidate = loc + move;
        if (const auto it = farm.open_tiles.find(candidate);
            it != farm.open_tiles.end() && explored.insert(candidate).second) {
          next_front.push_back(candidate);
          result[std::distance(farm.open_tiles.begin(), it)] = distance;
        }
      }
    }

    front = std::move(next_front);
  }

  return result;
}

auto solve_part3(const auto& input) {
  const auto tree_distances =
      std::views::transform(input.palm_trees,
                            [&](const Loc& tree) { return distances(input, tree); }) |
      std::ranges::to<std::vector>();

  const auto summed_tree_distances = std::ranges::fold_left(
      tree_distances, std::vector<Coord>(input.open_tiles.size()), [](auto&& acc, const auto& x) {
        for (auto&& [src, dst] : std::views::zip(x, acc)) {
          dst += src;
        }
        return acc;
      });

  auto candidates = std::views::zip(input.open_tiles, summed_tree_distances) |
                    std::views::filter([&](const auto& pair) {
                      return !input.palm_trees.contains(std::get<0>(pair));
                    }) |
                    std::views::elements<1>;

  return std::ranges::min(candidates);
}

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
