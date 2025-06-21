#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <istream>
#include <print>
#include <ranges>
#include <span>
#include <vector>

using Brightness = std::uint32_t;

auto parse_input(std::istream&& in) {
  return std::views::istream<Brightness>(in) | std::ranges::to<std::vector>();
}

auto solve_part1(const auto& input) {
  constexpr std::array<Brightness, 4> stamps{{1, 3, 5, 10}};

  return std::ranges::fold_left(
      std::views::transform(input,
                            [&](Brightness brightness) {
                              std::size_t result{};

                              for (const auto stamp : std::views::reverse(stamps)) {
                                const std::size_t count = brightness / stamp;
                                brightness -= (count * stamp);
                                result += count;
                              }

                              return result;
                            }),
      std::size_t{}, std::plus<>{});
}

auto memoize(const std::span<const Brightness> stamps, const Brightness upper_bound) {
  std::vector<std::size_t> result;
  result.reserve(upper_bound + Brightness{1});

  result.emplace_back();
  for (const auto brightness : std::views::iota(Brightness{1}, upper_bound + Brightness{1})) {
    result.emplace_back(std::ranges::min(
        std::views::filter(stamps, [&](const Brightness stamp) { return stamp <= brightness; }) |
        std::views::transform(
            [&](const Brightness stamp) { return result[brightness - stamp] + 1UZ; })));
  }

  return result;
}

auto solve_part2(const auto& input) {
  constexpr std::array<Brightness, 10> stamps{{1, 3, 5, 10, 15, 16, 20, 24, 25, 30}};

  const std::size_t upper_bound = *std::ranges::max_element(input);
  const std::vector<std::size_t> memory = memoize(stamps, upper_bound);

  return std::ranges::fold_left(
      std::views::transform(input, [&](Brightness brightness) { return memory[brightness]; }),
      std::size_t{}, std::plus<>{});
}

auto solve_part3(const auto& input) {
  constexpr std::array<Brightness, 18> stamps{
      {1, 3, 5, 10, 15, 16, 20, 24, 25, 30, 37, 38, 49, 50, 74, 75, 100, 101}};
  constexpr Brightness max_delta = 100;

  const std::size_t upper_bound =
      (*std::ranges::max_element(input) + max_delta + Brightness{1}) / Brightness{2};

  const std::vector<std::size_t> memory = memoize(stamps, upper_bound);

  return std::ranges::fold_left(
      std::views::transform(
          input,
          [&](Brightness brightness) {
            const Brightness low_brightness = brightness / Brightness{2};
            const Brightness high_brightness = brightness - low_brightness;
            const bool brightness_is_even = low_brightness == high_brightness;
            const Brightness upper_bound = brightness_is_even ? Brightness{50} : Brightness{49};

            return std::ranges::min(std::views::iota(Brightness{}, upper_bound + Brightness{1}) |
                                    std::views::transform([&](const Brightness imbalance) {
                                      return memory[low_brightness - imbalance] +
                                             memory[high_brightness + imbalance];
                                    }));
          }),
      std::size_t{}, std::plus<>{});
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
