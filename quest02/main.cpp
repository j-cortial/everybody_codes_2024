#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <functional>
#include <ios>
#include <istream>
#include <iterator>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

using namespace std::string_literals;

struct Input {
  std::vector<std::vector<char>> words;
  std::vector<std::vector<char>> sentences;
};

auto parse_input(std::istream&& in) {
  const auto data = std::views::istream<char>(in >> std::noskipws) | std::ranges::to<std::vector>();

  auto sections = std::views::split(data, "\n\n"s);
  auto current_section = sections.begin();

  auto word_section = *current_section++;
  auto tokens = std::views::split(
      std::views::filter(
          std::views::drop_while(word_section, [](const char c) { return c != ':'; }) |
              std::views::drop(1),
          [](const char c) { return c != '\n'; }),
      ',');
  auto words = tokens |
               std::views::transform([](auto&& rng) { return std::ranges::to<std::vector>(rng); }) |
               std::ranges::to<std::vector>();

  auto sentence_section = *current_section++;
  auto sentences = std::ranges::to<std::vector>(
      std::views::split(sentence_section, '\n') |
      std::views::filter([](const auto& rng) { return !rng.empty(); }) |
      std::views::transform([](auto&& rng) { return std::ranges::to<std::vector>(rng); }));

  return Input{.words = std::move(words), .sentences = std::move(sentences)};
}

auto occurence_count(const std::span<const char> needle, const std::span<const char> haystack) {
  std::size_t result{};

  auto remainder = std::ranges::subrange(haystack.begin(), haystack.end());
  for (;;) {
    const auto found = std::ranges::search(remainder, needle);
    if (found.begin() != found.end()) {
      result += 1UZ;
      remainder = std::ranges::subrange(std::next(found.begin()), remainder.end());
    } else {
      break;
    }
  }

  return result;
}

auto tag_occurences(const std::ranges::range auto& needle, const std::ranges::range auto& haystack,
                    std::ranges::range auto& tags) {
  auto zip = std::views::zip(haystack, tags);
  auto remainder = std::ranges::subrange(zip.begin(), zip.end());
  for (;;) {
    const auto found = std::ranges::search(remainder, needle, std::equal_to<>{},
                                           [](const auto& pair) { return std::get<0>(pair); });
    if (found.begin() != found.end()) {
      for (auto entry : found) {
        std::get<1>(entry) = true;
      }
      remainder = std::ranges::subrange(++found.begin(), remainder.end());
    } else {
      break;
    }
  }
}

auto solve_part1(const auto& input) {
  return std::ranges::fold_left(
      std::views::transform(
          input.sentences,
          [&](const auto& sentence) {
            return std::ranges::fold_left(
                std::views::transform(
                    input.words, [&](const auto& word) { return occurence_count(word, sentence); }),
                0UZ, std::plus<>{});
          }),
      0UZ, std::plus<>{});
}

auto solve_part2(const auto& input) {
  return std::ranges::fold_left(
      std::views::transform(input.sentences,
                            [&](const auto& sentence) {
                              std::vector<bool> tags(sentence.size(), false);
                              for (const auto& word : input.words) {
                                tag_occurences(word, sentence, tags);
                                tag_occurences(std::views::reverse(word), sentence, tags);
                              }
                              return std::ranges::count(tags, true);
                            }),
      0UZ, std::plus<>{});
}

auto solve_part3(const auto& input) {
  std::vector<std::vector<bool>> tags(input.sentences.size(),
                                      std::vector<bool>(input.sentences[0].size(), false));

  for (const auto& word : input.words) {
    const auto word_length = word.size();

    for (auto [sentence, tag_row] : std::views::zip(input.sentences, tags)) {
      std::array iotas{std::views::iota(0UZ, sentence.size()),
                       std::views::iota(0UZ, word_length - 1UZ)};
      auto haystack = std::views::join(iotas) |
                      std::views::transform([&](const std::size_t j) { return sentence[j]; });
      auto tag_range = std::views::join(iotas) |
                       std::views::transform([&](const std::size_t j) { return tag_row[j]; });
      tag_occurences(word, haystack, tag_range);
      tag_occurences(std::views::reverse(word), haystack, tag_range);
    }

    for (const std::size_t j : std::views::iota(0UZ, input.sentences[0].size())) {
      auto iota = std::views::iota(0UZ, input.sentences.size());
      auto haystack =
          iota | std::views::transform([&](const std::size_t i) { return input.sentences[i][j]; });
      auto tag_range =
          iota | std::views::transform([&](const std::size_t i) { return tags[i][j]; });
      tag_occurences(word, haystack, tag_range);
      tag_occurences(std::views::reverse(word), haystack, tag_range);
    }
  }

  return std::ranges::count(std::views::join(tags), true);
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
