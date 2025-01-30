import numpy as np
import random

# Define hyperparameters
embedding_dim = 5  # Number of dimensions in embedding space
window_size = 2  # Context window size
num_negative_samples = 5  # Negative samples per positive pair
learning_rate = 0.01
epochs = 10

# Sample corpus and vocabulary
corpus = ["the cat sat on the mat", "the dog barked at the cat"]
words = list(set(" ".join(corpus).split()))  # Unique words
word2idx = {word: i for i, word in enumerate(words)}
idx2word = {i: word for word, i in word2idx.items()}
V = len(word2idx)  # Vocabulary size

# Generate training data (center, [context_words])
data = []
for sentence in corpus:
    tokens = sentence.split()
    for center_idx, center_word in enumerate(tokens):
        context_range = list(range(max(0, center_idx - window_size), min(len(tokens), center_idx + window_size + 1)))
        context_range.remove(center_idx)  # Exclude center word
        context_words = [tokens[i] for i in context_range]
        data.append((word2idx[center_word], [word2idx[c] for c in context_words]))  # (center, [context words])

# Initialize word vectors
np.random.seed(42)
W_input = np.random.rand(V, embedding_dim)  # Input word embeddings
W_output = np.random.rand(V, embedding_dim)  # Output (context) embeddings

# Sigmoid function
def sigmoid(x):
    return 1 / (1 + np.exp(-x))

# Training loop
for epoch in range(epochs):
    total_loss = 0

    for center, context_words in data:  # Each center word has plural context words
        v_center = W_input[center]

        # Compute loss for all context words (positive samples)
        loss = 0
        for context in context_words:
            v_context = W_output[context]
            score = np.dot(v_center, v_context)
            loss += -np.log(sigmoid(score))  # Sum log prob for all context words

            # Compute gradient update for positive samples
            grad = sigmoid(score) - 1
            W_input[center] -= learning_rate * grad * v_context
            W_output[context] -= learning_rate * grad * v_center

        # Negative Sampling: Generate random words not in context
        negative_samples = random.sample(range(V), num_negative_samples)
        for negative in negative_samples:
            if negative in context_words:
                continue  # Avoid sampling true context words
            v_negative = W_output[negative]
            score = np.dot(v_center, v_negative)
            loss += -np.log(1 - sigmoid(score))  # Minimize prob of incorrect words

            # Compute gradient update for negative samples
            grad = sigmoid(score)
            W_input[center] -= learning_rate * grad * v_negative
            W_output[negative] -= learning_rate * grad * v_center

        total_loss += loss  # Accumulate loss for this iteration

    print(f"Epoch {epoch + 1}, Loss: {total_loss:.4f}")

