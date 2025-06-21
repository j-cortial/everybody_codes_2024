#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <istream>
#include <iterator>
#include <numeric>
#include <print>
#include <ranges>
#include <utility>
#include <vector>

auto parse_input(std::istream&& in) {
  std::size_t result;  // NOLINT(cppcoreguidelines-init-variables)
  in >> result;
  return result;
}

auto solve_part1(const auto& input) {
  const std::size_t complete_layer_count = std::floor(std::sqrt(input));
  const std::size_t final_layer_count = complete_layer_count * complete_layer_count == input
                                            ? complete_layer_count
                                            : complete_layer_count + 1UZ;
  const std::size_t missing_block_count = (final_layer_count * final_layer_count) - input;
  const std::size_t pyramid_width = (final_layer_count * 2UZ) - 1UZ;
  return missing_block_count * pyramid_width;
}

auto solve_part2(const auto& input) {
  constexpr std::size_t block_count = 20240000;
  constexpr std::size_t acolyte_count = 1111;

  std::size_t layer_count = 0;
  std::size_t used_block_count = 0;
  std::size_t next_layer_thickness = 1;

  while (used_block_count < block_count) {
    const std::size_t required_blocks = next_layer_thickness * ((2UZ * layer_count) + 1U);
    used_block_count += required_blocks;
    ++layer_count;
    next_layer_thickness = (next_layer_thickness * input) % acolyte_count;
  }

  const std::size_t missing_block_count = used_block_count - block_count;
  const std::size_t pyramid_width = (layer_count * 2UZ) - 1UZ;

  return missing_block_count * pyramid_width;
}

auto solve_part3(const auto& input) {
  constexpr std::size_t block_count = 202400000;
  constexpr std::size_t acolyte_count = 10;

  std::vector<std::size_t> column_heights;
  std::size_t next_layer_thickness = 1UZ;
  std::size_t max_used_block_count = 0;

  auto pyramid_width = [&]() {
    return column_heights.empty() ? 0UZ : (2UZ * column_heights.size()) - 1UZ;
  };

  for (;;) {
    column_heights.push_back(0UZ);
    std::ranges::transform(column_heights, column_heights.begin(),
                           [&](const std::size_t& v) { return v + next_layer_thickness; });

    const std::size_t required_blocks = next_layer_thickness * pyramid_width();
    max_used_block_count += required_blocks;

    const auto removed_block_counts =
        std::views::slide(column_heights, 2) | std::views::transform([&](const auto& rng) {
          const auto heights = rng | std::ranges::to<std::vector>();
          const std::size_t superfluous_block_count =
              ((input * pyramid_width()) * heights[0]) % acolyte_count;
          const std::size_t bound = heights[1] - 1UZ;
          return std::min(bound, superfluous_block_count);
        }) |
        std::ranges::to<std::vector>();

    const std::size_t removed_block_count =
        removed_block_counts.empty()
            ? 0UZ
            : removed_block_counts.front() +
                  (2UZ * std::reduce(std::next(removed_block_counts.begin()),
                                     removed_block_counts.end()));

    const std::size_t used_block_count = max_used_block_count - removed_block_count;
    if (used_block_count >= block_count) {
      return used_block_count - block_count;
    }

    next_layer_thickness = acolyte_count + (next_layer_thickness * input) % acolyte_count;
  }

  std::unreachable();
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
