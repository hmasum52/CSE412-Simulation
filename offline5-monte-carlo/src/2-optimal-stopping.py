# author: Hasan Masum
# id: 1805052


import numpy as np
import matplotlib.pyplot as plt
from tqdm import tqdm

# set the random seed
np.random.seed(52)

def simulate(n_candidates, m, s):
  # Generate a random permutation of the candidates
  candidates = np.random.permutation(n_candidates) + 1

  # Find the best candidate in the first m
  best_in_sample = np.min(candidates[:m]) if m > 0 else 0

  # Select the first candidate that is better than the best in the first ms
  for candidate in candidates[m:]:
    if candidate < best_in_sample: 
      # If any candidate is better, check if it's within top s
      return candidate <= s
  # If no candidate is better, return 0
  return False

def success_rate(n, m, s, num_simulations=10000):
  # Run the simulation multiple times and compute the success rate
  return np.mean([simulate(n, m, s) for _ in range(num_simulations)])

def main():
  # Population size
  # from which are going to select the best candidate
  n_candidates = 100

  # Success criteria
  success_criteria = [1, 3, 5, 10]

  # Sample sizes
  sample_group_sizes = np.arange(n_candidates) # list of sample group sizes from 0 to n-1

  # For each success criterion
  for s in success_criteria:
    print(f"Calculating Success rate for s = {s}")
    # Compute the success rate for each sample size
    success_rates = [
      success_rate(n_candidates, m, s) for m in tqdm(sample_group_sizes)
    ]
    # Plot the success rate vs. sample size
    plt.plot(sample_group_sizes, success_rates, label=f's = {s}')
    print("Simulation done for s = ", s, '\n')

  plt.xlabel('Sample group size (m)')
  plt.ylabel('Success rate')
  plt.legend()
  plt.show()

if __name__ == "__main__":
  main()