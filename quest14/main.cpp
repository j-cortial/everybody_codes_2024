#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <set>
#include <utility>
#include <vector>

enum class Direction : std::uint8_t { u, d, l, r, f, b };

constexpr std::array<Direction, 6> all_directions{
    Direction::u, Direction::d, Direction::l, Direction::r, Direction::f, Direction::b,
};

auto parse_direction(const char c) -> Direction {
  switch (c) {
    case 'U':
      return Direction::u;
    case 'D':
      return Direction::d;
    case 'L':
      return Direction::l;
    case 'R':
      return Direction::r;
    case 'F':
      return Direction::f;
    case 'B':
      return Direction::b;
    default:
      break;
  }
  std::unreachable();
}

using Length = std::int32_t;

struct Growth {
  Direction direction;
  Length length;
};

auto parse_input(std::istream&& in) {
  return std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
         std::views::filter([](const auto& line) { return line.begin() != line.end(); }) |
         std::views::transform([](const auto& line) {
           return std::views::lazy_split(line, ',') | std::views::transform([](const auto& rng) {
                    auto char_it = rng.begin();
                    const Direction direction = parse_direction(*char_it);
                    ++char_it;
                    const auto remainder =
                        std::ranges::subrange(char_it, rng.end()) | std::ranges::to<std::vector>();
                    Length length;  // NOLINT(cppcoreguidelines-init-variables)
                    std::from_chars(remainder.data(), std::next(remainder.data(), remainder.size()),
                                    length);
                    return Growth{.direction = direction, .length = length};
                  }) |
                  std::ranges::to<std::vector>();
         }) |
         std::ranges::to<std::vector>();
}

using Coord = std::int64_t;

auto vertical_growth(const Growth& growth) -> Coord {
  switch (growth.direction) {
    case Direction::u:
      return growth.length;
    case Direction::d:
      return -growth.length;
    default:
      break;
  }
  return {};
}

auto solve_part1(const auto& input) {
  return std::ranges::fold_left(input.front(), std::pair<Coord, Coord>{},
                                [](const auto& acc, const Growth& growth) {
                                  const auto new_height = acc.first + vertical_growth(growth);
                                  return std::make_pair(new_height,
                                                        std::max(new_height, acc.second));
                                })
      .second;
}

using Loc = std::array<Coord, 3>;

auto operator+(const Loc& a, const Loc& b) -> Loc {
  Loc result;
  std::ranges::transform(a, b, result.begin(), [](const Coord a_coord, const Coord b_coord) {
    return a_coord + b_coord;
  });
  return result;
}

auto base_vector(const Direction direction) -> Loc {
  switch (direction) {
    case Direction::u:
      return {0, 0, 1};
    case Direction::d:
      return {0, 0, -1};
    case Direction::l:
      return {0, -1, 0};
    case Direction::r:
      return {0, 1, 0};
    case Direction::f:
      return {1, 0, 0};
    case Direction::b:
      return {-1, 0, 0};
  }
  std::unreachable();
}

struct Tree {
  std::set<Loc> segments;
  std::set<Loc> leaves;

  static auto from_input(const auto& input) -> Tree;
};

auto Tree::from_input(const auto& input) -> Tree {
  Tree result;

  for (const auto& plant : input) {
    Loc loc{};
    for (const Growth& growth : plant) {
      const auto base = base_vector(growth.direction);
      for (const auto& _ : std::views::iota(Coord{0}, growth.length)) {
        loc = loc + base;
        result.segments.insert(loc);
      }
    }
    result.leaves.insert(loc);
  }

  return result;
}

auto solve_part2(const auto& input) { return Tree::from_input(input).segments.size(); }

auto murkiness(const Tree& tree, const Loc& start) -> Coord {
  Coord result{};

  std::set<Loc> explored{start};
  std::vector<Loc> front{start};

  std::size_t explored_leaves_count{};
  if (tree.leaves.contains(start)) {
    ++explored_leaves_count;
  }

  Coord distance{1};
  while (tree.leaves.size() != explored_leaves_count) {
    std::vector<Loc> next_front;

    for (const Loc& current : front) {
      for (const Direction dir : all_directions) {
        const Loc next = current + base_vector(dir);
        if (tree.segments.contains(next)) {
          if (explored.insert(next).second) {
            next_front.push_back(next);

            if (tree.leaves.contains(next)) {
              result += distance;
              ++explored_leaves_count;
            }
          }
        }
      }
    }

    front = std::move(next_front);
    ++distance;
  }

  return result;
}

auto solve_part3(const auto& input) {
  const auto tree = Tree::from_input(input);

  auto main_trunk = std::views::filter(tree.segments, [](const Loc& candidate) {
    return candidate[0] == Coord{} && candidate[1] == Coord{};
  });

  return std::ranges::min(
      std::views::transform(main_trunk, [&](const Loc& trunk) { return murkiness(tree, trunk); }));
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
