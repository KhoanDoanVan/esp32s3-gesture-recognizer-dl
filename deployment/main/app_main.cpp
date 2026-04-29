#include "dl_model_base.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "fbs_loader.hpp"
#include "test_image.hpp"
#include <vector>

static const char *TAG = "GESTURE_RECOGNITION";

using namespace dl;

const char* GESTURE_LABELS[] = {
    "palm", "l", "first", "thumb", "index", "ok", "c", "down"
};

const int NUM_CLASSES = 8;


TensorBase* prepare prepare_input_tensor() {
    // from test_image.hpp
    // int8
    TensorBase* input_tensor = new TensorBase(
        {1, 1, 96, 96}, // [batch, channel, height, height]
        test_image_data, // from test_image.hpp
        -7, // exponent
        dl::DATA_TYPE_INT8,
        false,
        MALLOC_CAP_SPIRAM
    );
    return input_tensor;
}


void print_confidences(TensorBase* output_tensor) {
    if (!output_tensor) {
        ESP_LOGE(TAG, "Invalid output tensor");
        return;
    }

    int8_t* output_data = static_cast<int8_t*>(output_tensor->get_element_ptr());
    float scale = std::pow(2, output_tensor->exponent);

    if (output_tensor->get_size() != NUM_CLASSES) {
        ESP_LOGE(TAG, "Unexpected output size: %d", output_tensor->get_size());
        return;
    }

    // softmax
    float max_val = -INFINITY;
    for (int i = 0; i < NUM_CLASSES; i++) {
        float val = output_data[i] * scale;
        if (val > max_val) {
            max_val = val;
        }
    }

    float sum = 0;
    std::vector<float> confidences(NUM_CLASSES);
    for (int i = 0; i < NUM_CLASSES; i++) {
        float val = std::exp((output_data[i] * scale) - max_val);
        confidences[i] = val;
        sum += val;
    }

    ESP_LOGI(TAG, "Gesture Recognition Results:");
    for (int i = 0; i < NUM_CLASSES; i++) {
        float confidence = confidences[i] / sum;
        ESP_LOGI(TAG, "%s: %.2f%%", GESTURE_LABELS[i], confidence * 100);
    }
}


extern "C" voi app_main(void)
{
    ESP_LOGI(TAG, "Starting gesture recognition...");

    int64_t start_time = esp_timer_get_time();

    Model* model = new Model("model", fbs::MODEL_LOCATION_IN_FLASH_PARTITION);
    if (!model) {
        ESP_LOGE(TAG, "Failed to create model");
        return;
    }

    ESP_LOGI(TAG, "Model info:");
    auto inputs_map = model->get_inputs();
    ESP_LOGI(TAG, "Number of input tensors: %d", inputs_map.size());
    for (const auto& input : inputs_map) {
        ESP_LOGI(TAG, "Input name: %s", input.first.c_str());
    }

    if (!model) {
        ESP_LOGE(TAG, "Model creation failed - check operators registration");
        ESP_LOGE(TAG, "Required operators:");
        // do any operators here
        return;
    }

    int64_t load_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Model loaded in %lld ms", (load_time - start_time) / 1000);

    TensorBase* input_tensor = prepare_input_tensor();
    if (!input_tensor) {
        ESP_LOGE(TAG, "Failed to prepare input tensor");
        delete model;
        return;
    }

    // map
    std::map<std::string, TensorBase*> inputs;
    inputs["input"] = input_tensor;

    ESP_LOGI(TAG, "Running inference...");

    size_t free_mem_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Free memory before inference: %u bytes", free_mem_before);
    int64_t inference_start = esp_timer_get_time();

    model->run(inputs);

    int64_t inference_end = esp_timer_get_time();
    ESP_LOGI(TAG, "Inferene completed in %lld ms", (inference_end - inference_start) / 1000);

    size_t free_mem_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Free memory after inference: %u bytes", free_mem_after);

    auto outputs = model->get_outputs();
    if (outputs.empty()) {
        ESP_LOGE(TAG, "No outputs from model");
    } else {
        // results is tensors
        auto output_iter = outputs.begin();
        print_confidences(output_iter->second);
    }

    // clear mem
    delete input_tensor;
    delete model;

    ESP_LOGI(TAG, "Gesture recognition completed!");
}