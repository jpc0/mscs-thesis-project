/*
   Implementation of the Aho-Corasick algorithm for multi-pattern matching.

   Unlike the single-pattern algorithms, this is not taken from prior art. This
   is coded directly from the algorithm pseudo-code in the Aho-Corasick paper.
   */

#include <queue>
#include <set>
#include <string>
#include <vector>
#include <string_view>
#include <numeric>

#include "run.hpp"

#if defined(__INTEL_LLVM_COMPILER)
#define LANG "cpp-intel"
#elif defined(__llvm__)
#define LANG "cpp-llvm"
#elif defined(__GNUC__)
#define LANG "cpp-gcc"
#endif

// Rather than implement a translation table for the four characters in the DNA
// alphabet, for now just let the alphabet be the full ASCII range and only use
// those four.
constexpr int ASIZE = 128;

// The "fail" value is used to determine certain states in the goto function.
constexpr int FAIL = -1;

/*
   For the creation of the failure function, we *would* loop over all of the
   values [0, ASIZE] looking for those that are non-fail. That would be very
   inefficient, given that our alphabet is actually just four characters. Use
   this array to shorten those loops.
   */
constexpr int OFFSETS_COUNT = 4;
static std::array<int, OFFSETS_COUNT> ALPHA_OFFSETS = {65, 67, 71, 84};

/*
   Enter the given pattern into the given goto-function, creating new states as
   needed. When done, add the index of the pattern into the partial output
   function for the state of the last character.
   */
void enter_pattern(std::string_view pat, int idx,
		std::vector<std::vector<int>> &goto_fn,
		std::vector<std::set<int>> &output_fn) {

	int len = pat.length();
	int j = 0; 
	int state = 0;
	static int new_state = 0;

	// Find the first leaf corresponding to a character in `pat`. From there is
	// where a new state (if needed) will be added.
	while (goto_fn[state][pat[j]] != FAIL) {
		state = goto_fn[state][pat[j]];
		j++;
	}

	// At this point, `state` points to the leaf in the automaton. Create new
	// states from here on for the remaining characters in `pat` that weren't
	// already in the automaton.
	for (int p = j; p < len; p++) {
		new_state++;
		goto_fn[state][pat[p]] = new_state;
		state = new_state;
	}

	output_fn[state].insert(idx);
}

/*
   Build the goto function and the (partial) output function.
   */
void build_goto(std::vector<std::string_view> pats, int num_pats,
		std::vector<std::vector<int>> &goto_fn,
		std::vector<std::set<int>> &output_fn) {
	int max_states = 0;

	// Calculate the maximum number of states as being the sum of the lengths of
	// patterns. This is overkill, but a more "serious" implementation would
	// have a more "serious" graph implementation for the goto function.
	/*
	max_states = std::accumulate(
			pats.begin(), 
			pats.end(), 
			0, 
			[](int init, std::string_view pat){return init += pat.length();}
			);
			*/
	for (int i = 0; i < num_pats; i++) {
		max_states += pats[i].length();
	}

	// Allocate for the goto function
	goto_fn.resize(max_states, std::vector<int>(ASIZE, FAIL));

	// Allocate for the output function
	output_fn.resize(max_states, std::set<int>());

	// OK, now actually build the goto function and output function.

	// Add each pattern in turn:
	for (int i = 0; i < num_pats; i++)
		enter_pattern(pats[i], i, goto_fn, output_fn);

	// Set the unused transitions in state 0 to point back to state 0:
	for (int i = 0; i < ASIZE; i++)
		if (goto_fn[0][i] == FAIL)
			goto_fn[0][i] = 0;
}

/*
   Build the failure function and complete the output function.
   */
std::vector<int> build_failure(std::vector<std::vector<int>> const &goto_fn,
		std::vector<std::set<int>> &output_fn) {
	// Need a simple queue of state numbers.
	std::queue<int> queue;

	// Allocate the failure function storage. This also needs to be as long as
	// goto_fn is, for safety.
	std::vector<int> failure_fn(goto_fn.size());

	// The queue starts out empty. Set it to be all states reachable from state 0
	// and set failure(state) for those states to be 0.
	for (int i = 0; i < OFFSETS_COUNT; i++) {
		int state = goto_fn[0][ALPHA_OFFSETS[i]];
		if (state == 0)
			continue;

		queue.push(state);
	}

	// This uses some single-letter variable names that match the published
	// algorithm. Their mnemonic isn't clear, or else I'd use more meaningful
	// names.
	while (!queue.empty()) {
		int r = queue.front();
		queue.pop();
		for (int i = 0; i < OFFSETS_COUNT; i++) {
			int a = ALPHA_OFFSETS[i];
			int s = goto_fn[r][a];
			if (s == FAIL)
				continue;

			queue.push(s);
			int state = failure_fn[r];
			while (goto_fn[state][a] == FAIL)
				state = failure_fn[state];
			failure_fn[s] = goto_fn[state][a];
			output_fn[s].insert(output_fn[s].begin(),
					output_fn[s].end());
		}
	}

	return failure_fn;
}

struct aho_corasic_return {
	int pattern_count;
	std::vector<std::vector<int>> goto_fn;
	std::vector<int> failure_fn;
	std::vector<std::set<int>> output_fn;
};

class aho_corasic : public MultiRunner<aho_corasic_return> {
	public:
		aho_corasic_return initializer(std::vector<std::string_view> patterns_data)
		{
			int patterns_count = patterns_data.size();

			// Initialize the multi-pattern structure.
			std::vector<std::vector<int>> goto_fn;
			std::vector<std::set<int>> output_fn;
			build_goto(patterns_data, patterns_count, goto_fn, output_fn);
			std::vector<int> failure_fn = build_failure(goto_fn, output_fn);

			return {
				patterns_count,
				goto_fn,
				failure_fn,
				output_fn
			};
		}

		std::vector<int> algorithm(aho_corasic_return const &pat_data, std::string_view const &sequence)
		{
			// Unpack pat_data
			int pattern_count = pat_data.pattern_count;
			auto goto_fn = pat_data.goto_fn;
			auto failure_fn = pat_data.failure_fn;
			auto output_fn = pat_data.output_fn;

			int state = 0;
			int n = sequence.length();
			std::vector<int> matches(pattern_count);

			for (int i = 0; i < n; i++) {
				while (goto_fn[state][sequence[i]] == FAIL) {
					state = failure_fn[state];
				}

				state = goto_fn[state][sequence[i]];
				for (auto output: output_fn[state]) {
					matches[output]++;
				}
			}

			return matches;
		}
};

int main(int argc, char* argv[]) {
	aho_corasic runner;
	int return_code = runner.run("aho_corasick", argc, argv);

	return return_code;
}
