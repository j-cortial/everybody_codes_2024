#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

using Number = std::uint16_t;
using Shout = std::uint64_t;

constexpr std::size_t column_count = 4UZ;

auto multipler(Number n) {
  Shout result{1};
  do {  // NOLINT(cppcoreguidelines-avoid-do-while)
    result *= Shout{10};
    n /= Number{10};
  } while (n > Number{});
  return result;
}

struct Floor {
  std::array<std::vector<Number>, column_count> columns;

  auto operator<=>(const Floor&) const = default;

  auto shout() const -> Shout {
    return std::ranges::fold_left(columns, Shout{}, [](const auto& acc, const auto& column) {
      const Number n = column.back();
      return n + (acc * multipler(n));
    });
  }

  auto dance(const std::size_t src_column_rank) {
    auto& src_column = columns[src_column_rank];
    const Number dancer_number = src_column.back();
    src_column.pop_back();

    auto& target_column = columns[(src_column_rank + 1UZ) % column_count];

    const std::size_t step_count = (dancer_number - 1) % (2 * target_column.size());
    const auto absorption_location =
        std::ptrdiff_t(step_count <= target_column.size() ? (target_column.size() - step_count)
                                                          : (step_count - target_column.size()));

    target_column.insert(std::next(target_column.begin(), absorption_location), dancer_number);
  }
};

auto parse_input(std::istream&& in) {
  const auto values = std::views::istream<Number>(in) | std::ranges::to<std::vector>();

  Floor result;

  for (auto&& rng : std::views::reverse(values) | std::views::chunk(column_count)) {
    for (auto&& [src, dst] : std::views::zip(rng, std::views::reverse(result.columns))) {
      dst.push_back(src);
    }
  }

  return result;
}

auto solve_part1(const auto& input) {
  auto floor = input;

  for (const auto round : std::views::iota(0UZ, 10UZ)) {
    const auto src_column_rank = round % column_count;
    floor.dance(src_column_rank);
  }

  return floor.shout();
}

auto solve_part2(const auto& input) {
  auto floor = input;

  std::unordered_map<Shout, std::size_t> counter;

  for (const auto round : std::views::iota(0UZ)) {
    const auto src_column_rank = round % column_count;
    floor.dance(src_column_rank);
    const auto shout = floor.shout();
    if (++counter[shout] == 2024UZ) {
      return shout * (round + 1UZ);
    }
  }

  std::unreachable();
}

auto solve_part3(const auto& input) {
  Shout result{};

  auto floor = input;
  std::set<Floor> memory{input};

  for (const auto round : std::views::iota(0UZ)) {
    const auto src_column_rank = round % column_count;
    floor.dance(src_column_rank);
    result = std::max(floor.shout(), result);
    if (src_column_rank == (column_count - 1UZ) && !memory.insert(floor).second) {
      return result;
    }
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
