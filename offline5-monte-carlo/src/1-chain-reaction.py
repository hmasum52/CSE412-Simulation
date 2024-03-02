# author: Hasan Masum
# id: 1805052

# Suppose pi is the probability that a neutron will result in a fission 
# that produces i new neutrons. Your task is to calculate the probability 
# distribution of the number of neutrons produced in the nth generation of a chain reaction.

# In this problem, a fissile neutron can only produce a maximum of 3 new neutrons, 
# while some may produce zero new ones. Here pi = (0.2126)(0.5893)i -1 for i = 1, 2, 3.
import numpy as np

def simulate_neutrons(probabilities, generations, num_simulations):
    print("Running simulation...")
    
    # Initialize the results array
    results = np.zeros((generations, 5))

    # For each generation
    for n in range(generations):
        print(f"Running Generation-{n+1}")
        # For each simulation
        for _ in range(num_simulations):
            # Start with one neutron
            num_neutrons = 1
            # For each generation
            for _ in range(n+1):
                # Determine the number of new neutrons produced
                new_neutrons = np.random.choice([0, 1, 2, 3], p=probabilities, size=num_neutrons)
                # Update the number of neutrons
                num_neutrons = np.sum(new_neutrons)
            # Update the results
            results[n, min(num_neutrons, 4)] += 1
        # Normalize the results to get probabilities
        results[n, :] /= num_simulations

    return results


def p_i(i: int):
    return 0.2126 * 0.5893 ** (i - 1)

# Define the probabilities
p = [p_i(i) for i in range(1, 4)] # probability for i = 1, 2, 3
p.insert(0, 1 - sum(p))  # probability for i = 0

# Simulate neutrons for 10 generations, 10,000 times
num_generations = 10
num_simulations = 10000
results = simulate_neutrons(p, num_generations, num_simulations)

# Print the results
for n in range(num_generations):
    print(f"Generation-{n+1}:")
    for j in range(5):
        print(f"p[{j}] = {results[n, j]:.4f}")