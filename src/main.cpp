#include <Wire.h>
#include <BH1750.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>

#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

BH1750 lightMeter;

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("BH1750 ready");
    } else {
        Serial.println("BH1750 not found");
        while (1);
    }
}

void loop() {
    float lux = lightMeter.readLightLevel();

    if (lux < 0) {
        Serial.println("Lỗi đọc cảm biến");
        delay(500);
        return;
    }

    Serial.print("Lux đo được: ");
    Serial.println(lux, 2);

    // Tạo signal cho Edge Impulse
    static float features[] = { lux };
    signal_t signal;
    signal.total_length = 1;
    signal.get_data = [](size_t offset, size_t length, float *out_ptr) -> int {
        out_ptr[0] = features[0];
        return 0;
    };

    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    if (res != EI_IMPULSE_OK) {
        Serial.print("Classification failed: ");
        Serial.println(res);
        return;
    }

    // Xử lý kết quả với 2 nhãn: "Sang", "Toi"
    const char* label_sang = ei_classifier_inferencing_categories[0];
    const char* label_toi = ei_classifier_inferencing_categories[1];

    float confidence_sang = result.classification[0].value;
    float confidence_toi = result.classification[1].value;

    Serial.println("=== Kết quả phân loại ===");
    Serial.print(label_sang); Serial.print(": "); Serial.println(confidence_sang, 3);
    Serial.print(label_toi);  Serial.print(": "); Serial.println(confidence_toi, 3);

    // In trạng thái dự đoán
    if (confidence_sang > confidence_toi) {
        Serial.println("==> Dự đoán: SÁNG");
    } else {
        Serial.println("==> Dự đoán: TỐI");
    }

    Serial.println("--------------------------");
    delay(1000);
}
