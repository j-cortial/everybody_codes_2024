#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>
#include <map>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

using CatFace = std::array<char, 3>;

using Offset = std::int32_t;

struct Wheel {
  std::vector<CatFace> faces;
  std::size_t step_size;

  auto visible_face(const std::size_t pull_rank, const Offset offset) const -> const CatFace& {
    const std::size_t positive_offset = [&]() {
      if (offset >= 0) {
        return std::size_t(offset);
      }
      const std::size_t multiplier = 1UZ + (faces.size() / std::size_t(-offset));
      return (faces.size() * multiplier) - std::size_t(-offset);
    }();
    return faces[((pull_rank * step_size) + positive_offset) % faces.size()];
  }

  auto visible_face(const std::size_t pull_rank) const -> const CatFace& {
    return visible_face(pull_rank, 0);
  }

  auto period() const -> std::size_t { return std::lcm(step_size, faces.size()) / step_size; }
};

using Input = std::vector<Wheel>;

auto parse_input(std::istream&& in) {
  auto data = std::views::istream<char>(in >> std::noskipws);
  auto lines = std::views::lazy_split(data, '\n');
  auto line_it = lines.begin();

  Input result = *line_it | std::views::lazy_split(',') | std::views::transform([](auto&& rng) {
    const auto token = rng | std::ranges::to<std::vector>();
    std::size_t step_size;  // NOLINT(cppcoreguidelines-init-variables)
    std::from_chars(token.data(), std::next(token.data(), token.size()), step_size);
    return Wheel{.faces = {}, .step_size = step_size};
  }) | std::ranges::to<std::vector>();

  for (auto&& line : std::ranges::subrange(std::ranges::next(line_it), lines.end())) {
    auto tokens = line | std::views::chunk(4) |
                  std::views::transform([](auto&& rng) { return rng | std::views::take(3); });
    for (auto&& [token, dst] : std::views::zip(tokens, result)) {
      if (*token.begin() != ' ') {
        std::ranges::copy(token, dst.faces.emplace_back().begin());
      }
    }
  }

  return result;
}

auto sequence(const Input& wheels, const std::size_t pull_rank, const Offset offset) {
  return std::views::join_with(std::views::transform(wheels,
                                                     [pull_rank, offset](const Wheel& wheel) {
                                                       return wheel.visible_face(pull_rank, offset);
                                                     }),
                               ' ');
}

auto sequence(const Input& wheels, const std::size_t pull_rank) {
  return sequence(wheels, pull_rank, 0);
}

auto solve_part1(const auto& input) {
  return sequence(input, 100UZ) | std::ranges::to<std::string>();
}

auto machine_period(const Input& wheels) -> std::size_t {
  return std::ranges::fold_left(wheels, 1UZ, [](const std::size_t acc, const auto& wheel) {
    return std::lcm(acc, wheel.period());
  });
}

auto bytes_earned(const Input& wheels, const std::size_t pull_rank, const Offset offset) {
  std::map<char, std::size_t> counts;
  for (char c : sequence(wheels, pull_rank, offset) | std::views::stride(2)) {
    ++counts[c];
  }

  return std::ranges::fold_left(std::views::values(counts), 0UZ,
                                [](const std::size_t acc, const std::size_t count) {
                                  return acc + (count >= 3UZ ? count - 2UZ : 0UZ);
                                });
}

auto bytes_earned(const Input& wheels, const std::size_t pull_rank) {
  return bytes_earned(wheels, pull_rank, 0);
}

auto solve_part2(const auto& input) {
  constexpr std::size_t pull_count = 202420242024UZ;

  const std::size_t period = machine_period(input);
  const std::size_t full_period_count = pull_count / period;
  const std::size_t remainder = pull_count % period;

  const std::size_t remainder_earnings =
      std::ranges::fold_left(std::views::iota(0UZ, remainder), 0UZ,
                             [&](const std::size_t acc, const std::size_t pull_rank) {
                               return acc + bytes_earned(input, pull_rank);
                             });

  const std::size_t period_earnings =
      std::ranges::fold_left(std::views::iota(remainder, period), remainder_earnings,
                             [&](const std::size_t acc, const std::size_t pull_rank) {
                               return acc + bytes_earned(input, pull_rank);
                             });

  return (period_earnings * full_period_count) + remainder_earnings;
}

auto solve_part3(const auto& input) {
  std::map<Offset, std::pair<std::size_t, std::size_t>> extreme_earnings{{0, {0UZ, 0UZ}}};

  for (auto pull_count : std::views::iota(0UZ, 256UZ)) {
    const std::size_t pull_rank = pull_count + 1UZ;
    std::map<Offset, std::pair<std::size_t, std::size_t>> next_extreme_earnings;

    for (const auto& [offset, values] : extreme_earnings) {
      const auto& [best, worst] = values;
      for (auto delta : std::array<Offset, 3>{{-1, 0, 1}}) {
        const Offset next_offset = offset + delta;
        const std::size_t new_earning = bytes_earned(input, pull_rank, next_offset);
        const std::size_t candidate_best = best + new_earning;
        const std::size_t candidate_worst = worst + new_earning;
        const auto [it, inserted] = next_extreme_earnings.emplace(
            next_offset, std::make_pair(candidate_best, candidate_worst));
        if (!inserted) {
          auto& pair = it->second;
          pair.first = std::max(pair.first, candidate_best);
          pair.second = std::min(pair.second, candidate_worst);
        }
      }
    }

    extreme_earnings = std::move(next_extreme_earnings);
  }

  const auto best_earning =
      std::ranges::max(std::views::values(extreme_earnings) | std::views::elements<0>);
  const auto worst_earning =
      std::ranges::min(std::views::values(extreme_earnings) | std::views::elements<1>);

  return std::format("{} {}", best_earning, worst_earning);
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
