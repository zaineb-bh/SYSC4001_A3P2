# SYSC4001_A3P2

**Compiling Part 2a:**
gcc Part2a.c -o part2a

**Running Part 2a:**
./part2a <number_of_TAs> <number_of_exams>

**Compiling Part B:**
gcc Part2b.c -o part2b

**Running Part 2b:**

./part2b <number_of_TAs> <number_of_exams>

**Test cases for Part 2a:**
./part2a.1 2 5      # minimum valid case

./part2a.2 3 10    # more exams

./part2a.3 4 5      # more TAs

./part2a.4 1 5      # If less than 2 TAs, program forces there to be 2 TAs

./part2a.5           # no number of processes/TAs passed, error


**Test cases for Part 2b:**

./part2b 2 5      # minimum valid case

./part2b 3 10    # more exams

./part2b 4 5      # more TAs

./part2b 1 5      # If less than 2 TAs, program forces there to be 2 TAs

./part2b           # no number of processes/TAs passed, error
