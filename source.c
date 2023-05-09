#include QMK_KEYBOARD_H

// Определение разных цветов для каждого слоя
static uint32_t layer_colors[4] = {
  0x000000, // Отключен
  0xFF0000, // Красный
  0x00FF00, // Зеленый
  0x0000FF, // Синий
};

// Обработчик события слоя
layer_state_t layer_state_set_user(layer_state_t state) {
  // Получаем текущий активный слой
  uint8_t layer = get_highest_layer(state);

  if (layer > 0) {
    // Устанавливаем цвет светодиода в зависимости от активного слоя
    set_led_color(layer_colors[layer]);
  } else {
    // Если слой отключен, отключаем светодиод
    set_led_color(0x000000);
  }

  // Возвращаем состояние слоя
  return state;
}

// Глобальные переменные для подсчета времени между нажатиями
static uint32_t last_minus_press_time = 0;
static uint8_t minus_press_count = 0;

// Обработчик нажатия клавиши "-"
void process_minus_press(uint16_t keycode, keyrecord_t *record) {
  // Проверяем, что клавиша нажата
  if (record->event.pressed) {
    uint32_t current_time = timer_read();

    // Проверяем, что это не повторный вызов функции
    if (current_time - last_minus_press_time > 1000) {
      // Сбрасываем счетчик нажатий
      if (minus_press_count == 4) {
        minus_press_count = 1;
      } else {
        minus_press_count++;
      }
    }

    if (minus_press_count == 4) {
      // Получаем текущий язык клавиатуры
      char *lang = get_keyboard_language();
      // Генерируем новую строку из символов "-"
      char str[20];
      memset(str, '-', 20);
      // Отправляем строку в операционную систему (на примере MacOS)
      send_string_to_os(str, lang);
    }

    // Сохраняем время нажатия
    last_minus_press_time = current_time;
  }
}

// Обработчик клавиши "-"
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case KC_MINUS:
      // Вызываем обработчик нажатия "-"
      process_minus_press(keycode, record);
      break;
    default:
      break;
  }

  // Возвращаем значение true, чтобы означать, что нажатие было обработано
  return true;
}
