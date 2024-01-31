#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"

// Definisikan nomor pin GPIO untuk masing-masing LED dan tombol
#define LED_R GPIO_NUM_25
#define LED_G GPIO_NUM_26
#define LED_B GPIO_NUM_27
#define BUTTON GPIO_NUM_0

// Tentukan waktu lama tekan (long press) dalam milidetik
#define LONG_PRESS_TIME_MS 2000

// Variabel global untuk menyimpan status LED
volatile int led_r_state = 0; // Status LED merah
volatile int led_b_state = 1; // Status LED biru (default menyala)

// Fungsi untuk mereset status LED ke kondisi awal
void reset_leds_to_default() {
    led_r_state = 0; // Matikan LED merah
    led_b_state = 1; // Nyalakan LED biru
    gpio_set_level(LED_R, led_r_state);
    gpio_set_level(LED_B, led_b_state);
}

// Task untuk kontrol LED
void led_task(void *pvParameter) {
    // Konfigurasi pin GPIO sebagai output untuk LED
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);

    // Atur status awal LED
    reset_leds_to_default();
    gpio_set_level(LED_G, 0); // Pastikan LED hijau dalam keadaan mati

    while (1) {
        // Toggle LED hijau setiap 300ms untuk menciptakan efek blinking
        gpio_set_level(LED_G, !gpio_get_level(LED_G)); // Ganti status LED hijau
        vTaskDelay(300 / portTICK_PERIOD_MS); // Tunda untuk periode blinking
    }
}

// Task untuk polling tombol
void button_task(void *pvParameter) {
    // Konfigurasi pin GPIO sebagai input untuk tombol
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON); // Aktifkan resistor pull-up internal

    int btn_press_time = 0; // Variabel untuk melacak lama waktu tekan tombol

    while (1) {
        if (gpio_get_level(BUTTON) == 0) { // Periksa jika tombol ditekan
            btn_press_time = 0; // Atur ulang penghitung waktu tekan

            // Tunggu sampai tombol dilepas atau waktu tekan lama tercapai
            while (gpio_get_level(BUTTON) == 0 && btn_press_time < LONG_PRESS_TIME_MS) {
                vTaskDelay(10 / portTICK_PERIOD_MS); // Tunda selama 10ms
                btn_press_time += 10; // Tambahkan ke penghitung waktu
            }

            // Periksa durasi tekanan tombol
            if (btn_press_time >= LONG_PRESS_TIME_MS) {
                // Deteksi tekanan lama, reset LED ke status awal
                reset_leds_to_default();
            } else {
                // Deteksi tekanan pendek, toggle status LED merah dan biru
                led_r_state = !led_r_state;
                led_b_state = !led_b_state;
                gpio_set_level(LED_R, led_r_state);
                gpio_set_level(LED_B, led_b_state);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Tunda polling untuk mengurangi bouncing
    }
}

// Fungsi utama yang dijalankan saat ESP32 dinyalakan
void app_main() {
    // Membuat task untuk mengontrol LED dan polling tombol
    xTaskCreate(&led_task, "led_task", 2048, NULL, 5, NULL);
    xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
}
