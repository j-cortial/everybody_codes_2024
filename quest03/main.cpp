#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <vector>

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;

auto operator+(const Loc& a, const Loc& b) -> Loc { return {a[0] + b[0], a[1] + b[1]}; }

using Grid = std::set<Loc>;

auto parse_input(std::istream&& in) {
  Grid result;

  auto chars = std::views::istream<char>(in >> std::noskipws);
  for (const auto& [i, row] : std::views::enumerate(std::views::lazy_split(chars, '\n'))) {
    for (const auto& [j, entry] : std::views::enumerate(row)) {
      if (entry == '#') {
        result.insert({Coord(i), Coord(j)});
      }
    }
  }

  return result;
}

using Depth = std::uint16_t;

class Mine {
 public:
  explicit Mine(const Grid& grid) {
    for (const auto& loc : grid) {
      depths_.emplace(loc, max_depth_);
    }
  }

  auto max_depth() const { return max_depth_; }

  auto depth_at(const Loc& loc) const -> Depth {
    const auto found = depths_.find(loc);
    if (found == depths_.end()) {
      return {};
    }
    return found->second;
  }

  template <auto moves>
  auto dig_one_layer() -> std::size_t {
    const std::vector<Loc> candidates =
        std::views::filter(depths_,
                           [&](const auto& entry) {
                             if (entry.second != max_depth()) {
                               return false;
                             }
                             return std::ranges::all_of(moves, [&](const Loc& move) {
                               return depth_at(entry.first + move) == max_depth_;
                             });
                           }) |
        std::views::transform([](const auto& entry) { return entry.first; }) |
        std::ranges::to<std::vector>();

    if (!candidates.empty()) {
      max_depth_ += Depth{1};
      for (const Loc& loc : candidates) {
        depths_[loc] = max_depth();
      }
    }

    return candidates.size();
  }

  auto cumulative_depth() const -> Depth {
    return std::ranges::fold_left(std::views::values(depths_), Depth{}, std::plus<>{});
  }

  static constexpr std::array<Loc, 4> basic_moves{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
  static constexpr std::array<Loc, 8> advanced_moves{
      {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}}};

 private:
  Depth max_depth_{1};
  std::map<Loc, Depth> depths_;
};

template <auto moves>
auto solve(const Grid& grid) {
  Mine mine{grid};

  while (mine.dig_one_layer<moves>() > 0) {
    // Nothing to do
  }

  return mine.cumulative_depth();
}

auto solve_part1(const auto& input) { return solve<Mine::basic_moves>(input); }

auto solve_part2(const auto& input) { return solve<Mine::basic_moves>(input); }

auto solve_part3(const auto& input) { return solve<Mine::advanced_moves>(input); }

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
