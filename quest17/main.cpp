#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <iterator>
#include <numeric>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;

auto parse_input(std::istream&& in) {
  return std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
         std::views::enumerate | std::views::transform([](auto&& row_pair) {
           auto [i, row] = row_pair;
           return row | std::views::enumerate |
                  std::views::filter([](auto&& col_pair) { return std::get<1>(col_pair) == '*'; }) |
                  std::views::transform([=](auto&& col_pair) {
                    auto [j, c] = col_pair;
                    return Loc{Coord(i), Coord(j)};
                  });
         }) |
         std::views::join | std::ranges::to<std::vector>();
}

auto distance(const Loc& a, const Loc& b) { return std::abs(a[0] - b[0]) + std::abs(a[1] - b[1]); }

using Edge = std::array<Loc, 2>;

auto length(const Edge& edge) { return distance(edge[0], edge[1]); }

auto constellation_size(const std::span<const Edge> constellation) {
  return std::ranges::fold_left(
             std::views::transform(constellation, [](const Edge& edge) { return length(edge); }),
             Coord{}, std::plus<>{}) +
         Coord(constellation.size() + 1UZ);
}

auto constellation_basis(const std::span<const Loc> stars) -> std::vector<Edge> {
  auto result = std::views::transform(
                    stars,
                    [&](const Loc& star) {
                      auto candidates =
                          std::views::filter(
                              stars, [&](const Loc& other_star) { return other_star != star; }) |
                          std::views::transform([&](const Loc& other_star) {
                            return std::make_pair(other_star, distance(star, other_star));
                          }) |
                          std::ranges::to<std::vector>();
                      std::ranges::sort(candidates, {},
                                        [](const auto& candidate) { return candidate.second; });
                      return std::make_pair(star, std::make_pair(candidates[0], candidates[1]));
                    }) |
                std::views::filter([](const auto& candidates) {
                  return candidates.second.first.second < candidates.second.second.second;
                }) |
                std::views::transform([](const auto& data) {
                  const Loc& a = data.first;
                  const Loc& b = data.second.first.first;
                  return Edge{std::min(a, b), std::max(a, b)};
                }) |
                std::ranges::to<std::vector>();

  std::ranges::sort(result);
  const auto [removed_begin, removed_end] = std::ranges::unique(result);
  result.erase(removed_begin, removed_end);

  return result;
}

auto find_star_group(std::vector<std::set<Loc>>& groups, const Loc& star) {
  return std::ranges::find_if(groups, [&](const auto& group) { return group.contains(star); });
}

auto merge_star_groups(std::vector<std::set<Loc>>& groups, const Edge& new_edge) {
  const auto first = find_star_group(groups, new_edge[0]);
  const auto second = find_star_group(groups, new_edge[1]);
  if (first != second) {
    first->merge(std::move(*second));
    *second = std::move(groups.back());
    groups.pop_back();
  }
}

auto compute_star_groups(const std::span<const Loc> stars, const std::span<const Edge> edges)
    -> std::vector<std::set<Loc>> {
  std::vector<std::set<Loc>> result =
      std::views::transform(stars, [](const Loc& star) { return std::set<Loc>{star}; }) |
      std::ranges::to<std::vector>();

  for (const Edge& edge : edges) {
    merge_star_groups(result, edge);
  }

  return result;
}

auto all_edges(const std::span<const Loc> stars) {
  return std::views::enumerate(stars) | std::views::transform([stars](const auto& pair) {
           const auto& [rank, star] = pair;
           return std::views::drop(stars, rank + 1UZ) |
                  std::views::transform([star](const Loc& other_star) {
                    return Edge{std::min(star, other_star), std::max(star, other_star)};
                  });
         }) |
         std::views::join;
}

auto constellation_edges(const std::span<const Loc> stars) -> std::vector<Edge> {
  if (stars.size() < 2UZ) {
    return {};
  }

  std::vector<Edge> result = constellation_basis(stars);
  auto groups = compute_star_groups(stars, result);

  while (result.size() != (stars.size() - 1UZ)) {
    const Edge best_candidate = std::ranges::min(
        all_edges(stars) | std::views::filter([&](const Edge& edge) {
          return find_star_group(groups, edge[0]) != find_star_group(groups, edge[1]);
        }),
        {}, length);
    merge_star_groups(groups, best_candidate);
    result.push_back(best_candidate);
  }

  return result;
}

auto solve_part1(const auto& input) { return constellation_size(constellation_edges(input)); }

auto solve_part2(const auto& input) { return constellation_size(constellation_edges(input)); }

auto partition_stars(const std::span<const Loc>& stars, const Coord threshold)
    -> std::vector<std::set<Loc>> {
  const auto valid_edges =
      all_edges(stars) |
      std::views::filter([threshold](const Edge& edge) { return length(edge) < threshold; }) |
      std::ranges::to<std::vector>();

  return compute_star_groups(stars, valid_edges);
}

auto solve_part3(const auto& input) {
  const auto partition = partition_stars(input, Coord{6});

  auto brilliant_constellations_sizes =
      std::views::transform(
          partition,
          [](const auto& stars) {
            return constellation_size(constellation_edges(std::ranges::to<std::vector>(stars)));
          }) |
      std::ranges::to<std::vector>();

  std::ranges::nth_element(brilliant_constellations_sizes,
                           std::next(brilliant_constellations_sizes.begin(), 3), std::greater<>{});
  return std::reduce(brilliant_constellations_sizes.begin(),
                     std::next(brilliant_constellations_sizes.begin(), 3), std::int64_t{1},
                     std::multiplies<>{});
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
