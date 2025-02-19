#ifndef LMS_H
#define LMS_H

#include <vector>

class LMSFilter {
    private:
        float mu; // Step size
        std::vector<float> input_buffer;
        std::vector<float> weights; // Filter weights

    public:
        // Initialize the NLMS filter
        LMSFilter(int order, float step_size);
        
        // Update the input buffer by removing the oldest sample and adding the new sample
        void updateInputBuffer(float new_sample);

        // Compute the filter output
        float computeFilterOutput();

        // Update the filter weights
        void updateWeights(float desired);

        // One iteration of the LMS filter
        float process(float input, float desired);
};

#endif