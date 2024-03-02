import numpy as np
import matplotlib.pyplot as plt

def simulate(n, m, s):
  # Generate a random permutation of the candidates
  candidates = np.random.permutation(n) + 1
  # Find the best candidate in the first m
  if m == 0:
    best_in_sample = 0
  else:
    best_in_sample = np.min(candidates[:m])
  # Select the first candidate that is better than the best in the first ms
  for candidate in candidates[m:]:
    if candidate < best_in_sample:
      # if s == 1 and candidate == n:
      #   return 1
      # elif s > 1 and candidate >= n - s + 1:
      #   return 1
      if candidate <= s:
        return 1
      else: return 0
  # If no candidate is better, return 0
  return 0

def success_rate(n, m, s, num_simulations=10000):
  # Run the simulation multiple times and compute the success rate
  return np.mean([simulate(n, m, s) for _ in range(num_simulations)])

# Population size
n_candidates = 100

# Success criteria
success_criteria = [1, 3, 5, 10]

# Sample sizes
sample_sizes = np.arange(n_candidates) # sample size from 0 to n-1

# For each success criterion
for s in success_criteria:
  # Compute the success rate for each sample size
  success_rates = [success_rate(n_candidates, m, s) for m in sample_sizes]
  # Plot the success rate vs. sample size
  plt.plot(sample_sizes, success_rates, label=f's = {s}')

plt.xlabel('Sample size (m)')
plt.ylabel('Success rate')
plt.legend()
plt.show()