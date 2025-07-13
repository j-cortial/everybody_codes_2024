#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;

using Hardness = std::uint8_t;

struct Target {
  Loc location;
  Hardness hardness;
};

auto hardness(char tile) -> Hardness {
  switch (tile) {
    case 'T':
      return Hardness{1};
    case 'H':
      return Hardness{2};
    default:
      break;
  }
  return {};
}

auto parse_input(std::istream&& in) {
  const auto data = std::views::istream<char>(in >> std::noskipws) | std::ranges::to<std::vector>();

  auto lines = std::views::split(data, '\n') |
               std::views::filter([](const auto& line) { return !line.empty(); });

  const auto line_count = std::ranges::distance(lines.begin(), lines.end());

  const auto first_line_coord = Coord(line_count - 2);

  auto rows = std::views::enumerate(lines) | std::views::transform([&](const auto& pair) {
                std::tuple result = pair;
                std::get<0>(result) = first_line_coord - std::get<0>(pair);
                return result;
              });

  auto targets =
      std::views::join(rows | std::views::transform([](const auto& row_pair) {
                         const auto& [y, row] = row_pair;
                         return std::views::enumerate(row) |
                                std::views::transform([y](const auto& col_pair) {
                                  const auto& [x, tile] = col_pair;
                                  return Target{{Coord(x - 1), Coord(y)}, hardness(tile)};
                                });
                       })) |
      std::views::filter([](const Target& target) { return target.hardness != Hardness{}; });

  return targets | std::ranges::to<std::vector>();
}

enum class Segment : std::uint8_t {
  a = 0,
  b = 1,
  c = 2,
};

using Power = Coord;
using Ranking = std::uint64_t;

struct Shot {
  Segment source;
  Power power;

  constexpr auto ranking() const -> Ranking {
    return Ranking(std::to_underlying(source) + 1) * Coord{power};
  }
};

auto fixed_target_solution(const Loc& target) -> Shot {
  const auto sum = target[0] + target[1];
  return {.source = Segment(sum % 3), .power = sum / 3};
}

auto solve_part1(const auto& input) {
  return std::ranges::fold_left(
      std::views::transform(input,
                            [](const Target& target) {
                              return fixed_target_solution(target.location).ranking() *
                                     target.hardness;
                            }),
      Ranking{}, std::plus<>{});
}

auto solve_part2(const auto& input) { return solve_part1(input); }

auto parse_input3(std::istream&& in) {
  return std::views::istream<Coord>(in) | std::views::chunk(2) |
         std::views::transform([](const auto& pair) {
           Loc result;
           std::ranges::copy(pair, result.begin());
           return result;
         }) |
         std::ranges::to<std::vector>();
}

auto moving_target_solution_ranking(const Loc& target) -> Ranking {
  const Coord launch_delay = target[0] % 2;
  const Coord hit_time = (target[0] + launch_delay) / 2;

  const Coord phase1_height = target[1] - target[0];
  assert(phase1_height <= Coord{2});  // Otherwise, there is no solution

  if (phase1_height >= 0) {
    return Shot{.source = Segment(phase1_height), .power = hit_time - launch_delay}.ranking();
  }

  const Coord remainder = (3 * target[0] + launch_delay) % 4;
  const Coord phase2_height_upper_bound =
      std::min(target[1] - ((3 * target[0] + launch_delay + remainder) / 4), 2);

  if (phase2_height_upper_bound >= 0) {
    return std::ranges::min(
        std::views::iota(Coord{}, phase2_height_upper_bound + 1) |
        std::views::transform([&](const auto segment) {
          return Shot{Segment(segment), target[1] - segment - hit_time}.ranking();
        }));
  }

  const Coord target_height_at_launch = target[1] - launch_delay;
  return Shot{.source = Segment(target_height_at_launch % 3), .power = target_height_at_launch / 3}
      .ranking();
}

auto solve_part3(const auto& input) {
  return std::ranges::fold_left(
      std::views::transform(input,
                            [](const Loc& initial_location) {
                              return moving_target_solution_ranking(initial_location);
                            }),
      Ranking{}, std::plus<>{});
}

auto main() -> int {
  const auto input1 = parse_input(std::ifstream{"input1.txt"});
  const auto answer1 = solve_part1(input1);
  std::println("The answer to part #1 is {}", answer1);

  const auto input2 = parse_input(std::ifstream{"input2.txt"});
  const auto answer2 = solve_part2(input2);
  std::println("The answer to part #2 is {}", answer2);

  const auto input3 = parse_input3(std::ifstream{"input3.txt"});
  const auto answer3 = solve_part3(input3);
  std::println("The answer to part #3 is {}", answer3);
}
