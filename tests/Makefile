CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I.

TEST_BINS = run_input_tests run_path_tests run_game_browser_tests run_screen_tests run_utils_tests run_screen_edge_tests

.PHONY: all clean test

all: $(TEST_BINS)

run_input_tests: test_switch_input.cpp switch_input_logic.h mock_sdl.h test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_switch_input.cpp

run_path_tests: test_switch_paths.cpp test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_switch_paths.cpp

run_game_browser_tests: test_game_browser.cpp game_browser_logic.h test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_game_browser.cpp

run_screen_tests: test_screen_scaling.cpp screen_logic.h test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_screen_scaling.cpp

run_utils_tests: test_utils.cpp test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_utils.cpp

run_screen_edge_tests: test_screen_edge_cases.cpp screen_logic.h test_framework.h
	$(CXX) $(CXXFLAGS) -o $@ test_screen_edge_cases.cpp

test: all
	@echo ""
	@echo "========================================"
	@echo "  Running ONScripter-Switch Unit Tests"
	@echo "========================================"
	@echo ""
	@failed=0; \
	for test in $(TEST_BINS); do \
		echo "--- Running $$test ---"; \
		./$$test || failed=1; \
		echo ""; \
	done; \
	if [ $$failed -eq 0 ]; then \
		echo "========================================"; \
		echo "  ALL TEST SUITES PASSED"; \
		echo "========================================"; \
	else \
		echo "========================================"; \
		echo "  SOME TESTS FAILED"; \
		echo "========================================"; \
		exit 1; \
	fi

clean:
	rm -f $(TEST_BINS)
