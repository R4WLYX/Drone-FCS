#pragma once
#include <vector>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <random>

struct Connection {
    int fromNeuron;
    float weight;
};

class Neuron {
public:
    std::vector<Connection> inputs;
    float output = 0.0f;
    float bias = 0.0f;
    float delta = 0.0f;

    Neuron() {
        bias = randomWeight();
    }

    void addInput(int fromIndex) {
        inputs.push_back({fromIndex, randomWeight()});
    }

    void removeInput(int fromIndex) {
        for (auto it = inputs.begin(); it != inputs.end(); ++it) {
            if (it->fromNeuron == fromIndex) {
                inputs.erase(it);
                break;
            }
        }
    }

    float activate(const std::vector<Neuron>& allNeurons) {
        float sum = bias;
        for (const auto& c : inputs) {
            sum += allNeurons[c.fromNeuron].output * c.weight;
        }
        output = tanh(sum); // Changed activation
        return output;
    }

    float activationDerivative() const {
        return 1.0f - (output * output); // derivative of tanh
    }

private:
    static float randomWeight() {
        return ((float)rand() / RAND_MAX - 0.5f) * 0.1f; // small init range
    }
};

class NeuralNet {
public:
    int inputCount;
    int outputCount;
    std::vector<Neuron> neurons;

    NeuralNet(int inputs, int outputs) : inputCount(inputs), outputCount(outputs) {
        for (int i = 0; i < inputs; ++i)
            neurons.push_back(Neuron());
        for (int i = 0; i < outputs; ++i)
            neurons.push_back(Neuron());
    }

    int addHiddenNeuron() {
        neurons.push_back(Neuron());
        return (int)neurons.size() - 1;
    }

    void removeNeuron(int index) {
        if (index < inputCount || index >= (int)neurons.size()) return;

        neurons.erase(neurons.begin() + index);

        for (auto& neuron : neurons) {
            for (auto it = neuron.inputs.begin(); it != neuron.inputs.end();) {
                if (it->fromNeuron == index) {
                    it = neuron.inputs.erase(it);
                } else {
                    if (it->fromNeuron > index) it->fromNeuron--;
                    ++it;
                }
            }
        }
    }

    void addConnection(int fromNeuron, int toNeuron) {
        if (!isValidNeuronIndex(fromNeuron) || !isValidNeuronIndex(toNeuron)) return;
        if (toNeuron < inputCount) return;

        neurons[toNeuron].addInput(fromNeuron);

        if (hasCycle()) {
            neurons[toNeuron].removeInput(fromNeuron);
        }
    }

    void removeConnection(int fromNeuron, int toNeuron) {
        if (!isValidNeuronIndex(toNeuron)) return;
        if (toNeuron < inputCount) return;
        neurons[toNeuron].removeInput(fromNeuron);
    }

    void setInput(const std::vector<float>& inputs) {
        if ((int)inputs.size() != inputCount) {
            std::cout << "FUUUUUUUUUUCK\n";
            return;
        }
        for (int i = 0; i < inputCount; ++i) {
            neurons[i].output = inputs[i];
        }
    }

    std::vector<float> forward() {
        std::vector<int> order;
        if (!topologicalSort(order)) {
            std::cerr << "Cycle detected, cannot forward\n";
            return {};
        }

        for (int idx : order) {
            if (idx >= inputCount) {
                neurons[idx].activate(neurons);
            }
        }

        std::vector<float> outputs;
        for (int i = (int)neurons.size() - outputCount; i < (int)neurons.size(); ++i) {
            outputs.push_back(neurons[i].output);
        }
        return outputs;
    }

    void mutateWeights(float mutationRate = 0.1f, float mutationStrength = 0.5f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> dist(0.0f, mutationStrength);
        std::uniform_real_distribution<float> prob(0.0f, 1.0f);

        for (auto& neuron : neurons) {
            if (prob(gen) < mutationRate) {
                neuron.bias += dist(gen);
            }

            for (auto& conn : neuron.inputs) {
                if (prob(gen) < mutationRate) {
                    conn.weight += dist(gen);
                }
            }
        }
    }

    int mutateAddNeuron() {
        return addHiddenNeuron();
    }

    void mutateAddConnection(float connectionProbability = 0.1f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> prob(0.0f, 1.0f);
        std::uniform_int_distribution<int> neuronIndexDist(0, (int)neurons.size() - 1);

        for (int toNeuron = inputCount; toNeuron < (int)neurons.size(); ++toNeuron) {
            if (prob(gen) < connectionProbability) {
                int fromNeuron = neuronIndexDist(gen);
                if (fromNeuron != toNeuron && fromNeuron < neurons.size()) {
                    addConnection(fromNeuron, toNeuron);
                }
            }
        }
    }

    void mutateRemoveNeuron(float removalProbability = 0.05f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> prob(0.0f, 1.0f);

        for (int i = (int)neurons.size() - outputCount - 1; i >= inputCount; --i) {
            if (prob(gen) < removalProbability) {
                removeNeuron(i);
            }
        }
    }

    void initializeHiddenLayers(int numLayers, int neuronsPerLayer) {
        if (numLayers <= 0 || neuronsPerLayer <= 0) return;

        std::vector<int> prevLayerIndices;
        for (int i = 0; i < inputCount; ++i) {
            prevLayerIndices.push_back(i);
        }

        for (int layer = 0; layer < numLayers; ++layer) {
            std::vector<int> currentLayerIndices;

            for (int i = 0; i < neuronsPerLayer; ++i) {
                int newNeuronIdx = addHiddenNeuron();
                currentLayerIndices.push_back(newNeuronIdx);
            }

            for (int currIdx : currentLayerIndices) {
                for (int prevIdx : prevLayerIndices) {
                    neurons[currIdx].addInput(prevIdx);
                }
            }

            prevLayerIndices = currentLayerIndices;
        }

        int firstOutputIdx = (int)neurons.size() - outputCount;
        for (int outIdx = firstOutputIdx; outIdx < (int)neurons.size(); ++outIdx) {
            for (int hiddenIdx : prevLayerIndices) {
                neurons[outIdx].addInput(hiddenIdx);
                if (hasCycle()) {
                    neurons[outIdx].removeInput(hiddenIdx);
                }
            }
        }
    }

    void train(const std::vector<float>& inputVals, const std::vector<float>& targetVals, float learningRate) {
        setInput(inputVals);

        std::vector<int> order;
        if (!topologicalSort(order)) {
            std::cerr << "Cycle detected - cannot train\n";
            return;
        }

        for (int idx : order) {
            if (idx >= inputCount)
                neurons[idx].activate(neurons);
        }

        int firstOutput = (int)neurons.size() - outputCount;
        for (int i = 0; i < outputCount; ++i) {
            int idx = firstOutput + i;
            float error = targetVals[i] - neurons[idx].output;
            neurons[idx].delta = error * neurons[idx].activationDerivative();
        }

        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            int idx = *it;
            if (idx < inputCount || idx >= firstOutput) continue;

            float sum = 0.0f;
            for (int j = firstOutput; j < (int)neurons.size(); ++j) {
                for (const auto& c : neurons[j].inputs) {
                    if (c.fromNeuron == idx) {
                        sum += neurons[j].delta * c.weight;
                    }
                }
            }
            neurons[idx].delta = sum * neurons[idx].activationDerivative();
        }

        for (int idx = inputCount; idx < (int)neurons.size(); ++idx) {
            Neuron& neuron = neurons[idx];
            for (auto& c : neuron.inputs) {
                float inputVal = neurons[c.fromNeuron].output;
                c.weight += learningRate * neuron.delta * inputVal;
            }
            neuron.bias += learningRate * neuron.delta;
        }
    }

private:
    bool isValidNeuronIndex(int idx) const {
        return idx >= 0 && idx < (int)neurons.size();
    }

    bool hasCycle() {
        std::vector<int> state(neurons.size(), 0);
        for (int i = 0; i < (int)neurons.size(); ++i) {
            if (dfsCycle(i, state)) return true;
        }
        return false;
    }

    bool dfsCycle(int node, std::vector<int>& state) {
        if (state[node] == 1) return true;
        if (state[node] == 2) return false;
        state[node] = 1;
        for (const auto& c : neurons[node].inputs) {
            if (dfsCycle(c.fromNeuron, state)) return true;
        }
        state[node] = 2;
        return false;
    }

    bool topologicalSort(std::vector<int>& order) {
        order.clear();
        std::vector<int> inDegree(neurons.size(), 0);
        for (int i = 0; i < (int)neurons.size(); ++i) {
            for (auto& c : neurons[i].inputs) {
                inDegree[i]++;
            }
        }

        std::vector<int> zeroDegree;
        for (int i = 0; i < (int)neurons.size(); ++i) {
            if (inDegree[i] == 0) zeroDegree.push_back(i);
        }

        while (!zeroDegree.empty()) {
            int curr = zeroDegree.back();
            zeroDegree.pop_back();
            order.push_back(curr);

            for (int j = 0; j < (int)neurons.size(); ++j) {
                for (auto& c : neurons[j].inputs) {
                    if (c.fromNeuron == curr) {
                        inDegree[j]--;
                        if (inDegree[j] == 0) zeroDegree.push_back(j);
                    }
                }
            }
        }
        return (order.size() == neurons.size());
    }
};