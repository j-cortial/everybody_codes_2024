#include <algorithm>
#include <cstddef>
#include <fstream>
#include <ios>
#include <istream>
#include <iterator>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace std::string_view_literals;

struct Branch {
  std::vector<std::string> children;
  std::size_t fruit_count{};
};

struct Tree {
  std::map<std::string, Branch, std::less<>> branches;

  auto fruit_paths() const -> std::vector<std::vector<std::string_view>>;
};

auto parse_input(std::istream&& in) {
  Tree result;

  auto branches = std::views::istream<char>(in >> std::noskipws) | std::views::lazy_split('\n') |
                  std::views::filter([](const auto& rng) { return rng.begin() != rng.end(); }) |
                  std::views::transform([](const auto& rng) {
                    auto sections = rng | std::views::lazy_split(':');

                    auto section_it = sections.begin();
                    auto name = *section_it | std::ranges::to<std::string>();

                    ++section_it;
                    auto children = *section_it | std::views::lazy_split(',') |
                                    std::views::transform([](const auto& rng) {
                                      return rng | std::ranges::to<std::string>();
                                    });

                    Branch branch;
                    for (std::string&& token : children) {
                      if (token == "@") {
                        branch.fruit_count += 1UZ;
                      } else {
                        branch.children.push_back(std::move(token));
                      }
                    }

                    return std::pair<std::string, Branch>{std::move(name), std::move(branch)};
                  });

  for (auto&& [name, branch] : branches) {
    result.branches.emplace(std::move(name), std::move(branch));
  }

  return result;
}

auto Tree::fruit_paths() const -> std::vector<std::vector<std::string_view>> {
  std::vector<std::vector<std::string_view>> result;

  std::vector<std::vector<std::string_view>> front{{"RR"sv}};

  while (!front.empty()) {
    const auto current = std::move(front.back());
    front.pop_back();

    const auto maybe_branch = branches.find(current.back());
    if (maybe_branch != branches.end()) {
      const auto& branch = maybe_branch->second;
      for (const auto& child : branch.children) {
        if (child != "BUG"sv && child != "ANT"sv) {
          auto next = current;
          next.push_back(child);
          front.emplace_back(std::move(next));
        }
      }
      if (branch.fruit_count != 0UZ) {
        auto next = current;
        next.push_back("@"sv);
        result.emplace_back(std::move(next));
      }
    }
  }

  return result;
}

auto unique_length_path(const Tree& tree) -> std::vector<std::string_view> {
  auto paths = tree.fruit_paths();
  const auto lengths =
      std::views::transform(
          paths,
          [](const auto& path_steps) {
            return std::ranges::fold_left(
                std::views::transform(path_steps,
                                      [](const std::string_view step) { return step.size(); }),
                0UZ, std::plus<>{});
          }) |
      std::ranges::to<std::vector>();

  std::map<std::size_t, std::size_t> counts;
  for (const auto length : lengths) {
    ++counts[length];
  }

  const auto unique_length =
      std::ranges::find_if(counts, [](const auto& entry) { return entry.second == 1UZ; })->first;

  return *std::next(paths.begin(),
                    std::distance(lengths.begin(), std::ranges::find(lengths, unique_length)));
}

auto solve_part1(const auto& input) {
  return std::views::join(unique_length_path(input)) | std::ranges::to<std::string>();
}

auto solve_part2(const auto& input) {
  return std::views::transform(unique_length_path(input),
                               [](const std::string_view& step) { return step[0]; }) |
         std::ranges::to<std::string>();
}

auto solve_part3(const auto& input) {
  return std::views::transform(unique_length_path(input),
                               [](const std::string_view& step) { return step[0]; }) |
         std::ranges::to<std::string>();
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
