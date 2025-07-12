#include <algorithm>
#include <array>
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

struct Input {
  std::vector<Target> targets;
};

auto parse_input(std::istream&& in) {
  const auto data = std::views::istream<char>(in >> std::noskipws) | std::ranges::to<std::vector>();

  auto lines = std::views::split(data, '\n') |
               std::views::filter([](const auto& line) { return !line.empty(); });

  const auto line_count = std::ranges::distance(lines.begin(), lines.end());

  const auto first_line_coord = Coord(line_count - 1);

  auto rows = std::views::enumerate(lines) | std::views::transform([&](const auto& pair) {
                auto result = pair;
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

  return Input{targets | std::ranges::to<std::vector>()};
}

enum class Segment : Coord {
  a = 1,
  b = 2,
  c = 3,
};

using Power = Coord;
using Ranking = std::uint64_t;

struct Shot {
  Segment source;
  Power power;

  constexpr auto ranking() const -> Ranking { return std::to_underlying(source) * Coord{power}; }
};

auto solution(const Loc& target) -> Shot {
  const auto sum = target[0] + target[1];
  return {.source = Segment((sum - Coord{1}) % 3 + Coord{1}), .power = (sum - Coord{1}) / 3};
}

auto solve_part1(const auto& input) {
  return std::ranges::fold_left(std::views::transform(input.targets,
                                                      [](const Target& target) {
                                                        return solution(target.location).ranking() *
                                                               target.hardness;
                                                      }),
                                Ranking{}, std::plus<>{});
}

auto solve_part2(const auto& input) { return solve_part1(input); }

auto solve_part3(const auto& input) { return 0; }

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
