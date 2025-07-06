#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

struct Engraving {
  std::array<std::array<char, 4>, 4> col_symbols;
  std::array<std::array<char, 4>, 4> row_symbols;
};

auto parse_input(std::istream&& in) {
  const auto data = std::views::istream<char>(in >> std::noskipws) | std::ranges::to<std::vector>();

  std::vector<Engraving> result;  // NOLINT(cppcoreguidelines-pro-type-member-init)

  auto lines = std::views::split(data, '\n') |
               std::views::filter([](const auto& rng) { return !rng.empty(); });

  auto all_rows =
      std::views::iota(0, 8) | std::views::transform([&lines](const auto rank) {
        return std::views::join_with(lines | std::views::drop(rank) | std::views::stride(8), ' ') |
               std::views::lazy_split(' ');
      });

  auto first_rows = *all_rows.begin();
  const auto count = std::ranges::distance(first_rows.begin(), first_rows.end());
  result.resize(count);

  for (auto [idx, sources] : std::views::enumerate(all_rows | std::views::take(2))) {
    auto targets = std::views::transform(result, [&](auto& engraving) {
      return std::views::transform(engraving.col_symbols,
                                   [&](auto& col) -> decltype(auto) { return col[idx]; });
    });
    for (auto [src, tgt] : std::views::zip(sources, targets)) {
      std::ranges::copy(src | std::views::drop(2) | std::views::take(4), tgt.begin());
    }
  }

  for (auto [idx, sources] :
       std::views::enumerate(all_rows | std::views::drop(2) | std::views::take(4))) {
    auto targets = std::views::transform(
        result, [&](auto& engraving) -> decltype(auto) { return engraving.row_symbols[idx]; });
    for (auto [src, tgt] : std::views::zip(sources, targets)) {
      std::ranges::copy(src | std::views::filter([](const char c) { return c != '.'; }),
                        tgt.begin());
    }
  }

  for (auto [idx, sources] : std::views::enumerate(all_rows | std::views::drop(6))) {
    auto targets = std::views::transform(result, [&](auto& engraving) {
      return std::views::transform(engraving.col_symbols,
                                   [&](auto& col) -> decltype(auto) { return col[idx + 2]; });
    });
    for (auto [src, tgt] : std::views::zip(sources, targets)) {
      std::ranges::copy(src | std::views::drop(2) | std::views::take(4), tgt.begin());
    }
  }

  return result;
}

auto runic_word(Engraving engraving) -> std::string {
  for (auto& row : engraving.row_symbols) {
    std::ranges::sort(row);
  }
  for (auto& col : engraving.col_symbols) {
    std::ranges::sort(col);
  }

  auto index_pairs =
      std::views::join(std::views::iota(0, 4) | std::views::transform([](const auto i) {
                         return std::views::iota(0, 4) | std::views::transform([i](const auto j) {
                                  return std::array{i, j};
                                });
                       }));

  return index_pairs | std::views::transform([&](const auto& indices) {
           const auto [i, j] = indices;
           char c;  // NOLINT(cppcoreguidelines-init-variables)
           std::ranges::set_intersection(engraving.row_symbols[i], engraving.col_symbols[j], &c);
           return c;
         }) |
         std::ranges::to<std::string>();
}

auto solve_part1(const auto& input) { return runic_word(input.front()); }

auto power(const std::string_view word) -> std::size_t {
  return std::ranges::fold_left(std::views::enumerate(word), std::size_t{},
                                [&](const std::size_t acc, const auto x) {
                                  const auto& [i, c] = x;
                                  return acc + ((i + 1) * (c - 'A' + 1));
                                });
}

auto solve_part2(const auto& input) {
  return std::ranges::fold_left(
      std::views::transform(
          input, [](const Engraving& engraving) { return power(runic_word(engraving)); }),
      std::size_t{}, std::plus<>{});
}

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
