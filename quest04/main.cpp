#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <vector>

using NailLength = std::uint64_t;

auto parse_input(std::istream&& in) {
  return std::views::istream<NailLength>(in) | std::ranges::to<std::vector>();
}

auto solve_part1(const auto& input) {
  const NailLength base_length = *std::ranges::min_element(input);
  return std::ranges::fold_left(
      std::views::transform(input, [&](const NailLength length) { return length - base_length; }),
      NailLength{}, std::plus<>());
}

auto solve_part2(const auto& input) { return solve_part1(input); }

auto solve_part3(const auto& input) {
  const std::size_t nail_count = input.size();
  auto nails = input;
  const auto midpoint = std::next(nails.begin(), nail_count / 2);
  std::ranges::nth_element(nails.begin(), midpoint, nails.end());
  const NailLength target_length = *midpoint;

  return std::ranges::fold_left(
             std::views::transform(std::ranges::subrange(nails.begin(), std::next(midpoint)),
                                   [&](const NailLength length) { return target_length - length; }),
             NailLength{}, std::plus<>()) +
         std::ranges::fold_left(
             std::views::transform(std::ranges::subrange(std::next(midpoint), nails.end()),
                                   [&](const NailLength length) { return length - target_length; }),
             NailLength{}, std::plus<>());
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
