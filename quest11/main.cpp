#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

using Category = std::string;
using LifeCycle = std::map<Category, std::vector<Category>>;

auto parse_input(std::istream&& in) {
  auto entries = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
                 std::views::filter([](const auto& line) { return line.begin() != line.end(); }) |
                 std::views::transform([](const auto& line) {
                   auto parts = std::views::lazy_split(line, ':');

                   auto part_it = parts.begin();
                   const Category parent = *part_it | std::ranges::to<std::string>();

                   ++part_it;
                   auto children = std::views::lazy_split(*part_it, ',') |
                                   std::views::transform([](const auto& token) {
                                     return token | std::ranges::to<std::string>();
                                   }) |
                                   std::ranges::to<std::vector>();

                   return std::make_pair(parent, std::move(children));
                 });

  LifeCycle result;

  for (auto&& entry : entries) {
    result.insert(std::move(entry));
  }

  return result;
}

using Population = std::map<Category, std::size_t>;

auto breed(const LifeCycle& life_cycle, const Population& current) -> Population {
  Population result;
  for (const auto& [parent, count] : current) {
    const auto& children = life_cycle.find(parent)->second;
    for (const auto& child : children) {
      result[child] += count;
    }
  }
  return result;
}

auto breed(const LifeCycle& life_cycle, Population population, const std::size_t cycles)
    -> Population {
  for (auto _ : std::views::iota(0UZ, cycles)) {
    population = breed(life_cycle, population);
  }
  return population;
}

auto population_count(const LifeCycle& life_cycle, const Population& initial_population,
                      const std::size_t cycles) -> std::size_t {
  const Population population = breed(life_cycle, initial_population, cycles);
  return std::ranges::fold_left(std::views::values(population), std::size_t{}, std::plus<>{});
}

auto solve_part1(const auto& input) { return population_count(input, {{"A", 1UZ}}, 4UZ); }

auto solve_part2(const auto& input) { return population_count(input, {{"Z", 1UZ}}, 10UZ); }

auto solve_part3(const auto& input) {
  const auto [min, max] = std::ranges::minmax(
      std::views::keys(input) | std::views::transform([&input](const Category& category) {
        return population_count(input, {{category, 1UZ}}, 20UZ);
      }));
  return max - min;
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
