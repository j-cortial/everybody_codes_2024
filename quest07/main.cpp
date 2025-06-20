#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <map>
#include <optional>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

enum class Instruction : std::uint8_t {
  increase,
  decrease,
  maintain,
};

auto instruction_from_char(char c) -> std::optional<Instruction> {
  switch (c) {  // NOLINT(bugprone-switch-missing-default-case)
    case '+':
      return Instruction::increase;
    case '-':
      return Instruction::decrease;
    case '=':
    case 'S':
      return Instruction::maintain;
    default:
      return {};
  }
  std::unreachable();
}

using PlanId = char;

using Plan = std::vector<Instruction>;

auto parse_input(std::istream&& in) {
  auto plans = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
               std::views::filter([](const auto& rng) { return rng.begin() != rng.end(); }) |
               std::views::transform([](const auto& rng) {
                 auto sections = rng | std::views::lazy_split(':');

                 auto section_it = sections.begin();
                 const PlanId plan_id = *(*section_it).begin();

                 ++section_it;
                 auto plan = std::views::lazy_split(*section_it, ',') |
                             std::views::transform([](const auto& rng) {
                               return *instruction_from_char(*rng.begin());
                             }) |
                             std::ranges::to<std::vector>();
                 return std::pair<PlanId, Plan>{plan_id, std::move(plan)};
               });

  std::map<PlanId, Plan> result;
  for (auto&& [plan_id, plan] : plans) {
    result.emplace(plan_id, std::move(plan));
  }
  return result;
}

using Energy = std::uint64_t;

auto operator+(const Energy e, const Instruction i) {
  switch (i) {
    case Instruction::increase:
      return Energy(e + Energy{1});
    case Instruction::decrease:
      return (e > Energy{}) ? Energy(e - Energy{1}) : e;
    case Instruction::maintain:
      return e;
  };
  std::unreachable();
}

auto collected_energy(const Plan& plan) {
  auto instructions =
      std::views::join(std::views::repeat(std::views::all(plan))) | std::views::take(10);

  return std::ranges::fold_left(instructions, std::pair{Energy{10}, Energy{}},
                                [](const auto acc, const Instruction x) {
                                  const auto [current, sum] = acc;
                                  const Energy next = current + x;
                                  return std::pair{next, sum + next};
                                })
      .second;
}

auto solve_part1(const auto& input) {
  auto v = std::views::transform(input,
                                 [](const auto& entry) {
                                   const auto& [plan_id, plan] = entry;
                                   return std::pair{plan_id, collected_energy(plan)};
                                 }) |
           std::ranges::to<std::vector>();

  std::ranges::sort(v, std::greater<>{}, [](const auto& entry) { return entry.second; });

  return std::views::transform(v, [](const auto& entry) { return entry.first; }) |
         std::ranges::to<std::string>();
}

using Track = std::vector<Instruction>;

using Coord = std::int16_t;
using Loc = std::array<Coord, 2>;

constexpr std::array<Loc, 4> moves{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

auto operator+(const Loc a, const Loc b) { return Loc{Coord(a[0] + b[0]), Coord(a[1] + b[1])}; }

struct TrackData {
  std::vector<std::vector<std::optional<Instruction>>> data;

  auto tile(const Loc& loc) const -> std::optional<Instruction> {
    if (loc[0] < Coord(0) || loc[0] >= Coord(data[0].size()) || loc[1] < Coord(0) ||
        loc[1] >= Coord(data.size())) {
      return {};
    }
    return data[loc[1]][loc[0]];
  }
};

auto parse_track(std::istream&& in) {
  const TrackData data{
      std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
      std::views::filter([](const auto& rng) { return rng.begin() != rng.end(); }) |
      std::views::transform([](const auto& rng) {
        return rng | std::views::transform([](const char c) { return instruction_from_char(c); }) |
               std::ranges::to<std::vector>();
      }) |
      std::ranges::to<std::vector>()};

  Track result;

  Loc current{1, 0};
  Loc previous{0, 0};
  while (current != Loc{0, 0}) {
    result.push_back(*data.tile(current));
    const auto move = *std::ranges::find_if(moves, [&](const Loc m) {
      const auto candidate = current + m;
      return candidate != previous && data.tile(candidate).has_value();
    });
    previous = std::exchange(current, current + move);
  }
  result.push_back(*data.tile(current));

  return result;
}

auto actual_instruction(const Instruction tile, const Instruction instruction) {
  return tile == Instruction::maintain ? instruction : tile;
}

auto collected_energy(const Track& track, const Plan& plan, const std::size_t loops) {
  auto tiles = std::views::join(std::views::repeat(std::views::all(track), loops));
  auto instructions = std::views::join(std::views::repeat(std::views::all(plan)));
  auto steps = std::views::zip(tiles, instructions);

  return std::ranges::fold_left(steps, std::pair{Energy{10}, Energy{}},
                                [](const auto acc, const auto step) {
                                  const auto [tile, instruction] = step;
                                  const auto [current, sum] = acc;
                                  const Energy next =
                                      current + actual_instruction(tile, instruction);
                                  return std::pair{next, sum + next};
                                })
      .second;
}

auto solve_part2(const auto& input, const auto& track) {
  auto v = std::views::transform(input,
                                 [&](const auto& entry) {
                                   const auto& [plan_id, plan] = entry;
                                   return std::pair{plan_id, collected_energy(track, plan, 10)};
                                 }) |
           std::ranges::to<std::vector>();

  std::ranges::sort(v, std::greater<>{}, [](const auto& entry) { return entry.second; });

  return std::views::transform(v, [](const auto& entry) { return entry.first; }) |
         std::ranges::to<std::string>();
}

auto solve_part3(const auto& input, const auto& track) {
  const Energy threshold = collected_energy(track, input.begin()->second, 11);

  Plan plan{
      Instruction::increase, Instruction::increase, Instruction::increase, Instruction::increase,
      Instruction::increase, Instruction::decrease, Instruction::decrease, Instruction::decrease,
      Instruction::maintain, Instruction::maintain, Instruction::maintain,
  };

  std::size_t result{};
  do {  // NOLINT(cppcoreguidelines-avoid-do-while)
    if (collected_energy(track, plan, 11) > threshold) {
      ++result;
    }
  } while (std::ranges::next_permutation(plan).found);

  return result;
}

auto main() -> int {
  const auto input1 = parse_input(std::ifstream{"input1.txt"});
  const auto answer1 = solve_part1(input1);
  std::println("The answer to part #1 is {}", answer1);

  const auto input2 = parse_input(std::ifstream{"input2.txt"});
  const auto track2 = parse_track(std::ifstream{"track2.txt"});
  const auto answer2 = solve_part2(input2, track2);
  std::println("The answer to part #2 is {}", answer2);

  const auto input3 = parse_input(std::ifstream{"input3.txt"});
  const auto track3 = parse_track(std::ifstream{"track3.txt"});
  const auto answer3 = solve_part3(input3, track3);
  std::println("The answer to part #3 is {}", answer3);
}
