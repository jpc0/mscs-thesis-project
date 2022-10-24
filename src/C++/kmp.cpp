/*
  Implementation of the Knuth-Morris-Pratt algorithm.

  This is based heavily on the code given in chapter 7 of the book, "Handbook
  of Exact String-Matching Algorithms," by Christian Charras and Thierry Lecroq.
*/

#include <iostream>
#include <string>
#include <vector>
#include <string_view>

#include "run.hpp"

/*
  Initialize the jump-table that KMP uses.
*/
void make_next_table(std::string_view pat, int m, std::vector<int> &next_table) {
  int i; 
  int j;

  i = 0;
  j = next_table[0] = -1;

  while (i < m) {
    while (j > -1 && pat[i] != pat[j])
      j = next_table[j];

    i++;
    j++;

    if (pat[i] == pat[j])
      next_table[i] = next_table[j];
    else
      next_table[i] = j;
  }

  return;
}

struct kmp_return {
	std::string pattern;
	std::vector<int> next_table;
};

class kmp : public Runner<kmp_return> {
public:
	kmp_return initializer(std::string_view pattern) override {
		int m = pattern.length();
		// Set up the next_table array for the algorithm to use:
		std::vector<int> next_table;
		next_table.reserve(m+1);
		// Set up a copy of pattern, with the sentinel character added:
		auto pat = std::string(pattern);
		pat.append("\0");
		make_next_table(pat, m, next_table);

		
		return kmp_return{std::move(pat),std::move(next_table)};
	};

	int algorithm(kmp_return const& pat_data, std::string_view const& sequence) override {
		int i;
		int	j;
		int matches = 0;

		// Unpack pat_data:
		std::string_view pattern = pat_data.pattern;
		std::vector<int> const& next_table = pat_data.next_table;

		// Get the size of the pattern and the sequence.
		int m = pattern.length();
		int n = sequence.length();

		// Perform the searching:
		i = 0;
		j = 0;
		while (j < n) {
			while (i > -1 && pattern[i] != sequence[j])
				i = next_table[i];

			i++;
			j++;
			if (i >= m) {
				matches++;
				i = next_table[i];
			}
		}

		return matches;
	};
};

int main(int argc, char *argv[]) {
  kmp runner;
  int return_code = runner.run("kmp", argc, argv);

  return return_code;
}
