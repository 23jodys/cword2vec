import numpy as np
import random
from collections import defaultdict

# Sample small dataset
corpus = ["the cat sat on the mat", "the dog barked at the cat"]

# Tokenize and build vocabulary
words = set(" ".join(corpus).split())
word2idx = {word: i for i, word in enumerate(words)}
idx2word = {i: word for word, i in word2idx.items()}

# Generate training data
window_size = 2
data = []

for sentence in corpus:
    tokens = sentence.split()
    for center_idx, center_word in enumerate(tokens):
        context_range = list(range(max(0, center_idx - window_size), min(len(tokens), center_idx + window_size + 1)))
        context_range.remove(center_idx)  # Exclude center word
        for context_idx in context_range:
            data.append((word2idx[center_word], word2idx[tokens[context_idx]]))  # (center, context)

# Initialize embedding matrices
embedding_size = 5
V = len(word2idx)
W_input = np.random.rand(V, embedding_size)
W_output = np.random.rand(V, embedding_size)

# Sigmoid function for negative sampling loss
def sigmoid(x):
    return 1 / (1 + np.exp(-x))

# Training hyperparameters
learning_rate = 0.01
epochs = 1000
num_negative_samples = 5

# Training loop
for epoch in range(epochs):
    loss = 0
    for center, context in data:
        # Positive sample (center, correct context)
        v_center = W_input[center]
        v_context = W_output[context]
        score = np.dot(v_center, v_context)
        loss += -np.log(sigmoid(score))  # Maximize positive probability

        # Negative Sampling: Generate random words
        negative_samples = random.sample(range(V), num_negative_samples)
        for negative in negative_samples:
            if negative == context:
                continue  # Avoid sampling true context word
            v_negative = W_output[negative]
            score = np.dot(v_center, v_negative)
            loss += -np.log(1 - sigmoid(score))  # Minimize probability of incorrect words

        # Gradient Updates (Stochastic Gradient Descent)
        grad = sigmoid(score) - 1
        W_input[center] -= learning_rate * grad * v_context
        W_output[context] -= learning_rate * grad * v_center

    # Print loss every 100 epochs
    if epoch % 100 == 0:
        print(f"Epoch {epoch}, Loss: {loss:.4f}")

# Output learned word embeddings
print("\nFinal Word Embeddings:")
for word, idx in word2idx.items():
    print(f"{word}: {W_input[idx]}")
