#include <cctype>
#include <fstream>
#include <iostream>
#include <istream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> read_file(std::string file_name) {
  std::vector<std::string> words; // 定义存储字符串的vector容器
  std::string line, word;

  // 打开文件
  std::ifstream file(file_name);

  // 逐行读取文件内容
  while (getline(file, line)) {
    // 使用字符串流按空格分割单词
    std::istringstream iss(line);
    while (iss >> word) {
      // 将分割后的单词存储到vector容器中
      words.push_back(word);
    }
  }

  return words;
}

int main() {
  auto file_name = "../range/wc.txt";
  auto v = read_file(file_name);
  auto res = v | std::views::transform([](const std::string &s) {
               return s | std::views::transform([](char c) -> char {
                        if (c >= 'A' && c <= 'Z')
                          return c - 26;
                        return c;
                      });
             });
  // for (auto s : res) {
  //   std::cout << s << std::endl;
  // }
}