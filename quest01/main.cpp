#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <istream>
#include <print>
#include <ranges>
#include <utility>
#include <vector>

auto parse_input(std::istream&& in) {
  return std::views::istream<char>(in) |
         std::views::filter([](const char c) { return isalpha(c); }) |
         std::ranges::to<std::vector>();
}

auto required_potions(const char c) {
  switch (c) {
    case 'x':
      [[fallthrough]];
    case 'A':
      return 0UZ;
    case 'B':
      return 1UZ;
    case 'C':
      return 3UZ;
    case 'D':
      return 5UZ;
    default:
      std::unreachable();
  }
}

auto solve(const auto& input, const std::size_t group_size) {
  return std::ranges::fold_left(
      std::views::transform(
          std::views::chunk(input, group_size),
          [](auto&& group) {
            const auto monsters = std::ranges::to<std::vector>(group);
            const auto actual_monster_count = monsters.size() - std::ranges::count(monsters, 'x');
            const auto additional_strength =
                (actual_monster_count * actual_monster_count) - actual_monster_count;
            return std::ranges::fold_left(std::views::transform(monsters, required_potions), 0UZ,
                                          std::plus<>{}) +
                   additional_strength;
          }),
      0UZ, std::plus<>{});
}

auto solve_part1(const auto& input) { return solve(input, 1UZ); }

auto solve_part2(const auto& input) { return solve(input, 2UZ); }

auto solve_part3(const auto& input) { return solve(input, 3UZ); }

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
