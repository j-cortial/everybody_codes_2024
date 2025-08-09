#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <istream>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

enum class Rotation : std::uint8_t {
  clockwise,
  counterclockwise,
};

auto rotation_from_char(const char c) -> Rotation {
  switch (c) {
    case 'L':
      return Rotation::counterclockwise;
    case 'R':
      return Rotation::clockwise;
    default:
      break;
  }
  std::unreachable();
}

using Coord = std::int32_t;
using Loc = std::array<Coord, 2>;

class Grid {
 public:
  Grid(std::vector<char> data, std::size_t width) : width_{width}, data_{std::move(data)} {}

  auto width() const -> std::size_t { return width_; }
  auto height() const -> std::size_t { return data_.size() / width_; };

  auto inside() const {
    return std::views::iota(Coord{1}, Coord(height() - 1UZ)) |
           std::views::transform([&](const auto j) {
             return std::views::iota(Coord{1}, Coord(width_ - 1UZ)) |
                    std::views::transform([j](const auto i) { return Loc{Coord(i), Coord(j)}; });
           }) |
           std::views::join;
  }

  auto data(const Loc& loc) const -> char { return data_[index(loc)]; }
  auto data(const Loc& loc) -> char& { return data_[index(loc)]; }

  auto rotate_around(const Loc& center, Rotation rot) -> void;

  auto message() const -> std::optional<std::string>;

 private:
  auto index(const Loc& loc) const -> std::size_t {
    return std::size_t(loc[0UZ]) + (std::size_t(loc[1UZ]) * width_);
  }

  std::size_t width_;
  std::vector<char> data_;
};

struct Input {
  std::vector<Rotation> instructions;
  Grid grid;
};

auto parse_input(std::istream&& in) {
  auto lines = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
               std::views::filter([](const auto& line) { return line.begin() != line.end(); });

  auto line_it = lines.begin();

  auto instruction_line = *line_it;
  auto instructions =
      instruction_line | std::views::transform(rotation_from_char) | std::ranges::to<std::vector>();

  auto grid_lines = std::ranges::subrange(line_it, lines.end());

  std::vector<char> grid_data;
  std::size_t grid_width{};
  for (auto [j, c] : grid_lines | std::views::transform([](auto&& line) {
                       return line | std::views::enumerate;
                     }) | std::views::join) {
    grid_data.push_back(c);
    grid_width = std::max(grid_width, std::size_t(j) + 1UZ);
  }

  return Input{
      .instructions = std::move(instructions),
      .grid = {std::move(grid_data), grid_width},
  };
}

constexpr std::array<Loc, 8> neighbors{
    {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}}};

constexpr auto operator+(const Loc& a, const Loc& b) -> Loc { return {a[0] + b[0], a[1] + b[1]}; }

auto Grid::rotate_around(const Loc& center, const Rotation rot) -> void {
  auto neighborhood = std::views::transform(
      neighbors, [&](const Loc& delta) -> decltype(auto) { return data(center + delta); });
  switch (rot) {
    case Rotation::clockwise: {
      for (auto&& [a, b] : neighborhood | std::views::reverse | std::views::pairwise) {
        std::swap(a, b);
      }
    } break;
    case Rotation::counterclockwise: {
      for (auto&& [a, b] : neighborhood | std::views::pairwise) {
        std::swap(a, b);
      }
    } break;
  }
}

auto Grid::message() const -> std::optional<std::string> {
  const auto open = std::ranges::find(data_, '>');
  if (open == data_.end()) {
    return std::nullopt;
  }
  const auto close = std::ranges::find(open, data_.end(), '<');
  if (close == data_.end()) {
    return std::nullopt;
  }
  const std::size_t open_line_rank = std::ranges::distance(data_.begin(), open) / width_;
  const std::size_t close_line_rank = std::ranges::distance(data_.begin(), close) / width_;
  if (open_line_rank != close_line_rank) {
    return std::nullopt;
  }
  return {std::ranges::subrange(std::next(open), close) | std::ranges::to<std::string>()};
}

auto decrypt(Grid grid, const std::span<const Rotation> instructions, const std::uint32_t rounds) {
  for ([[maybe_unused]] auto round : std::views::iota(std::uint32_t{}, rounds)) {
    for (auto&& [center, rot] : std::views::zip(
             grid.inside(), std::views::repeat(std::views::all(instructions)) | std::views::join)) {
      grid.rotate_around(center, rot);
    }
  }
  return grid.message().value_or("<NO MESSAGE>");
}

auto solve_part1(const auto& input) { return decrypt(input.grid, input.instructions, 1); }

auto solve_part2(const auto& input) { return decrypt(input.grid, input.instructions, 100); }

auto solve_part3(const auto& input) { return decrypt(input.grid, input.instructions, 0); }

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
