# Руководство

## Кастомизация
### Как настроить поведение вашей клавиатуры

Для многих людей настраиваемая клавиатура — это больше, чем отправка нажатий кнопок на ваш компьютер. Вы хотите иметь возможность выполнять более сложные действия, чем простое нажатие кнопок и макросы. В QMK есть хуки, которые позволяют вам вводить код, переопределять функциональность и иным образом настраивать поведение вашей клавиатуры в различных ситуациях.

Эта страница не предполагает каких-либо специальных знаний о QMK, но чтение [Понимание QMK](https://docs.qmk.fm/#/understanding_qmk) поможет вам понять, что происходит на более фундаментальном уровне.

### Слово о ядре, клавиатурах и раскладке

Мы структурировали QMK в виде иерархии:

	+ Ядро (_квант)
		+ Клавиатура/версия (_kb)
			+ Карта ключей (_user)
Каждая из описанных ниже функций может быть определена с суффиксом `_kb()` или суффиксом `_user()`. Мы предполагаем, что вы будете использовать суффикс `_kb()` на уровне клавиатуры/редакции, а суффикс `_user()` следует использовать на уровне раскладки клавиатуры.

При определении функций на уровне клавиатуры/ревизии важно, чтобы ваша реализация `_kb()` вызывала `_user()` перед выполнением чего-либо еще, иначе функция уровня раскладки никогда не будет вызвана.

### Пользовательские коды клавиш

На сегодняшний день наиболее распространенной задачей является изменение поведения существующего кода клавиши или создание нового кода клавиши. С точки зрения кода механизм для каждого очень похож.

### Определение нового кода ключа

Первым шагом к созданию собственного кода(ов) клавиш является их перечисление. Это означает как присвоение им имени, так и присвоение уникального номера этому коду клавиши. Вместо того, чтобы ограничивать пользовательские коды клавиш фиксированным диапазоном чисел, QMK предоставляет макрос `SAFE_RANGE`. Вы можете использовать `SAFE_RANGE` при перечислении ваших пользовательских кодов клавиш, чтобы гарантировать, что вы получите уникальный номер.

Вот пример перечисления 2 кодов клавиш. После добавления этого блока в ваш `keymap.c` вы сможете использовать `FOO` и `BAR` внутри вашего `keymap`.

```
enum my_keycodes {
  FOO = SAFE_RANGE,
  BAR
};
```

### Программирование поведения любого кода клавиши

Если вы хотите переопределить поведение существующего ключа или определить поведение для нового ключа, вы должны использовать функции `process_record_kb()` и `process_record_user()`. Они вызываются QMK во время обработки ключа до того, как будет обработано фактическое событие ключа. Если эти функции вернут `true`, QMK будет обрабатывать коды клавиш как обычно. Это может быть удобно для расширения функциональности ключа, а не для его замены. Если эти функции возвращают `false`, QMK пропустит обычную обработку клавиш, и вам придется отправлять любые требуемые события нажатия или нажатия клавиши.

Эти функции вызываются каждый раз при нажатии или отпускании клавиши.

### Пример реализации `process_record_user()`

Этот пример делает две вещи. Он определяет поведение пользовательского кода клавиши `FOO` и дополняет нашу клавишу `Enter`, воспроизводя звук при каждом ее нажатии.

```
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case FOO:
      if (record->event.pressed) {
        // Do something when pressed
      } else {
        // Do something else when release
      }
      return false; // Skip all further processing of this key
    case KC_ENTER:
      // Play a tone when enter is pressed
      if (record->event.pressed) {
        PLAY_SONG(tone_qwerty);
      }
      return true; // Let QMK send the enter press/release events
    default:
      return true; // Process all other keycodes normally
  }
}
```

### Документация по функции process_record_*

Клавиатура/Редакция 	: `bool process_record_kb(uint16_t keycode, keyrecord_t *record)`
Раскладка 				: `bool process_record_user(uint16_t keycode, keyrecord_t *record)`
Аргумент `keycode` — это то, что определено в вашей раскладке, например `MO(1)`, `KC_L` и т. д. Для обработки этих событий следует использовать блок `switch...case`.

Аргумент записи содержит информацию о фактическом прессе:
```
keyrecord_t record {
  keyevent_t event {
    keypos_t key {
      uint8_t col
      uint8_t row
    }
    bool     pressed
    uint16_t time
  }
}
```

### Код инициализации клавиатуры

Процесс инициализации клавиатуры состоит из нескольких шагов. В зависимости от того, что вы хотите сделать, это повлияет на то, какую функцию вы должны использовать.

Это три основные функции инициализации, перечисленные в порядке их вызова.

`keyboard_pre_init_*` 		— происходит до того, как что-либо будет запущено. Хорошо подходит для настройки оборудования, которое вы хотите запустить очень рано.
`matrix_init_*` 			— происходит в середине процесса запуска прошивки. Аппаратное обеспечение инициализировано, но фич может еще не быть.
`keyboard_post_init_*` 	 	— происходит в конце процесса запуска прошивки. Это то место, где вы, по большей части, хотели бы поместить код «настройки».

Для большинства людей функция `keyboard_post_init_user` — это то, что вы хотите вызвать. Например, здесь вы хотите настроить `RGB Underglow`.

### Код предварительной инициализации клавиатуры

Это запускается очень рано во время запуска, даже до того, как USB был запущен.

Вскоре после этого матрица инициализируется.

Для большинства пользователей это не следует использовать, так как это в первую очередь для аппаратно-ориентированной инициализации.

Однако, если у вас есть оборудование, которое вам нужно инициализировать, это лучшее место для него (например, инициализация светодиодных контактов).

### Пример реализации keyboard_pre_init_user()

В этом примере на уровне клавиатуры B0, B1, B2, B3 и B4 устанавливаются в качестве контактов светодиода.
```
void keyboard_pre_init_user(void) {
  // Call the keyboard pre init code.

  // Set our LED pins as output
  setPinOutput(B0);
  setPinOutput(B1);
  setPinOutput(B2);
  setPinOutput(B3);
  setPinOutput(B4);
}
```

### Keyboard Post Initialization code

This is ran as the very last task in the keyboard initialization process. This is useful if you want to make changes to certain features, as they should be initialized by this point.

#### Example keyboard_post_init_user() Implementation

This example, running after everything else has initialized, sets up the rgb underglow configuration.
```
void keyboard_post_init_user(void) {
  // Call the post init code.
  rgblight_enable_noeeprom(); // enables Rgb, without saving settings
  rgblight_sethsv_noeeprom(180, 255, 255); // sets the color to teal/cyan without saving
  rgblight_mode_noeeprom(RGBLIGHT_MODE_BREATHING + 3); // sets mode to Fast breathing without saving
}

```

#### keyboard_post_init_* Function Documentation

Keyboard/Revision: void keyboard_post_init_kb(void)
Keymap: void keyboard_post_init_user(void)

### Matrix Scanning Code

Whenever possible you should customize your keyboard by using process_record_*() and hooking into events that way, to ensure that your code does not have a negative performance impact on your keyboard. However, in rare cases it is necessary to hook into the matrix scanning. Be extremely careful with the performance of code in these functions, as it will be called at least 10 times per second.

#### [Example matrix_scan_* Implementation](https://docs.qmk.fm/#/custom_quantum_functions?id=example-matrix_scan_-implementation)

This example has been deliberately omitted. You should understand enough about QMK internals to write this without an example before hooking into such a performance sensitive area. If you need help please open an issue or chat with us on Discord.

#### [matrix_scan_* Function Documentation()](https://docs.qmk.fm/#/custom_quantum_functions?id=matrix_scan_-function-documentation)

Keyboard/Revision: void matrix_scan_kb(void)
Keymap: void matrix_scan_user(void)
Эта функция вызывается при каждом сканировании матрицы, что, по сути, происходит так часто, как может обрабатывать MCU. Будьте осторожны с тем, что вы помещаете сюда, так как это будет много работать.

Вы должны использовать эту функцию, если вам нужен собственный код сканирования матрицы. Его также можно использовать для пользовательского вывода состояния (например, светодиодов или дисплея) или других функций, которые вы хотите регулярно запускать, даже когда пользователь не печатает.

### Уборка клавиатуры

Клавиатура/Редакция: void housekeeping_task_kb(void)
Карта ключей: void housekeeping_task_user(void)

Эта функция вызывается в конце всей обработки QMK перед началом следующей итерации. Можно смело предположить, что QMK разобрался с последним сканированием матрицы на момент вызова этих функций — состояния слоев были обновлены, отчеты USB отправлены, светодиоды обновлены, дисплеи нарисованы.

Как и `matrix_scan_*`, они вызываются так часто, как может обрабатывать MCU. Чтобы поддерживать отзывчивость вашей доски, рекомендуется делать как можно меньше во время этих вызовов функций, потенциально ограничивая их поведение, если вам действительно требуется реализовать что-то особенное.

#### Пример реализации void housekeeping_task_user(void)

В этом примере показано, как использовать `void housekeeping_task_user(void)` для отключения RGB-подсветки. Для матрицы RGB следует использовать встроенный параметр `RGB_MATRIX_TIMEOUT`.

Во-первых, добавьте следующие строки в `config.h` вашей раскладки клавиатуры:

```
#define RGBLIGHT_SLEEP  // разрешить rgblight_suspend() и rgblight_wakeup() в файле keymap.c
#define RGBLIGHT_TIMEOUT 900000  // сколько ms ждать пока rgblight таймаута, 900K ms is 15 min.
```

Затем добавьте следующий код в файл `keymap.c`:
```
static uint32_t key_timer;           // таймер для последней активности клавиатуры, используйте 32-битное значение и функцию, чтобы сделать возможным более длительное время простоя
static void refresh_rgb(void);       // обновляет таймер активности и RGB, вызывается всякий раз, когда происходит какое-либо действие
static void check_rgb_timeout(void); // проверяет, прошло ли достаточно времени для тайм-аута RGB
bool is_rgb_timeout = false;         // сохранить, если время RGB истекло или нет в логическом значении

void refresh_rgb(void) {
    key_timer = timer_read32(); 	// сохранить время последнего обновления
    if (is_rgb_timeout)
    {
        is_rgb_timeout = false;
        rgblight_wakeup();
    }
}
void check_rgb_timeout(void) {
    if (!is_rgb_timeout && timer_elapsed32(key_timer) > RGBLIGHT_TIMEOUT) // проверьте, не истек ли тайм-аут RGB и прошло ли достаточно времени
    {
        rgblight_suspend();
        is_rgb_timeout = true;
    }
}

// Затем вызовите вышеуказанные функции из встроенных в QMK функций постобработки, например:
// Запускается в конце каждого цикла сканирования, проверяет, не произошло ли время ожидания RGB или нет

void housekeeping_task_user(void) {
#ifdef RGBLIGHT_TIMEOUT
    check_rgb_timeout();
#endif
}


/* Запускается после каждого нажатия клавиши, проверьте, произошла ли активность */
void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef RGBLIGHT_TIMEOUT
    if (record->event.pressed)
        refresh_rgb();
#endif
}


/* Запускается после каждого тика энкодера, проверяет, произошла ли активность */
void post_encoder_update_user(uint8_t index, bool clockwise) {
#ifdef RGBLIGHT_TIMEOUT
    refresh_rgb();
#endif
}
```

### Код холостого хода/пробуждения клавиатуры

Если плата это поддерживает, то ее можно «заморозить», остановив ряд функций. Хорошим примером этого является RGB-подсветка или подсветка. Это может сэкономить энергопотребление или улучшить поведение вашей клавиатуры.

Этим управляют две функции: `suspend_power_down_*` и `suspend_wakeup_init_*`, которые вызываются, когда системная плата бездействует и когда она просыпается соответственно.

### Отложенное выполнение

QMK имеет возможность выполнять обратный вызов через указанный период времени, вместо того, чтобы вручную управлять таймерами. Чтобы включить эту функциональность, установите `DEFERRED_EXEC_ENABLE = yes` в `rules.mk`.


### Расширенные темы

Эта страница раньше включала в себя большой набор функций. Мы переместили многие разделы, которые раньше были частью этой страницы, на отдельные страницы. Все, что ниже этой точки, является просто перенаправлением, чтобы люди, переходящие по старым ссылкам в Интернете, находили то, что ищут.


### Layer Change Code

This runs code every time that the layers get changed. This can be useful for layer indication, or custom layer handling.

#### Пример реализации `layer_state_set_*`

This example shows how to set the RGB Underglow lights based on the layer, using the Planck as an example.


```
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case _RAISE:
        rgblight_setrgb (0x00,  0x00, 0xFF);
        break;
    case _LOWER:
        rgblight_setrgb (0xFF,  0x00, 0x00);
        break;
    case _PLOVER:
        rgblight_setrgb (0x00,  0xFF, 0x00);
        break;
    case _ADJUST:
        rgblight_setrgb (0x7A,  0x00, 0xFF);
        break;
    default: //  for any other layers, or the default layer
        rgblight_setrgb (0x00,  0xFF, 0xFF);
        break;
    }
  return state;
}
```
Use the `IS_LAYER_ON_STATE(state, layer)` and `IS_LAYER_OFF_STATE(state, layer)` macros to check the status of a particular layer.

Outside of `layer_state_set_*` functions, you can use the `IS_LAYER_ON(layer)` and `IS_LAYER_OFF(layer)` macros to check global layer state.


#### Документация по функции `layer_state_set_*`

Keyboard/Revision: 	`layer_state_t layer_state_set_kb(layer_state_t state)`
Keymap: 			`layer_state_t layer_state_set_user(layer_state_t state)`
The state is the bitmask of the active layers, as explained in the [Keymap Overview](https://docs.qmk.fm/#/keymap?id=keymap-layer-status)



















## Слои
One of the most powerful and well used features of QMK Firmware is the ability to use layers. For most people, this amounts to a function key that allows for different keys, much like what you would see on a laptop or tablet keyboard.

For a detailed explanation of how the layer stack works, checkout [Keymap Overview](https://docs.qmk.fm/#/keymap?id=keymap-and-layers).


### Переключение слоев

Эти функции позволяют активировать слои различными способами. Обратите внимание, что слои, как правило, не являются независимыми макетами — несколько слоев могут быть активированы одновременно, и для слоев типично использовать `KC_TRNS`, чтобы позволить нажатиям клавиш проходить на более низкие уровни. При использовании мгновенного переключения слоев с помощью `MO()`, `LM()`, `TT()` или `LT()` убедитесь, что ключ на вышеуказанных слоях оставлен прозрачным, иначе он может работать не так, как предполагалось.

- DF(layer) - переключает слой по умолчанию. Слой по умолчанию — это всегда активный базовый слой, поверх которого накладываются другие слои. Подробнее о слое по умолчанию см. ниже. Это может быть использовано для переключения с раскладки QWERTY на раскладку Дворака. (Обратите внимание, что это временный переключатель, который сохраняется только до тех пор, пока клавиатура не отключится. Для постоянного изменения слоя по умолчанию требуется более глубокая настройка, например, вызов функции `set_single_persistent_default_layer` внутри `process_record_user`.)
- МО(layer) - моментально активирует слой. Как только вы отпустите клавишу, слой деактивируется.
- LM (layer, мод) - Мгновенно активирует слой (как МО), но с активным модификатором(ами). Поддерживает только слои 0-15. Модификаторы, которые принимает этот код клавиши, имеют префикс `MOD_`, а не `KC_`. Эти модификаторы можно комбинировать с помощью побитового ИЛИ, например. `LM(_RAISE, MOD_LCTL | MOD_LALT)`.
 - LT(layer, kc) - моментально активирует слой при удержании и отправляет kc при нажатии. Поддерживает только слои 0-15.
- OSL (layer) - моментально активирует слой, пока не будет нажата следующая клавиша. Подробности и дополнительные функции см. в разделе One Shot Keys.
- TG(layer) - переключает слой, активируя его, если он неактивен и наоборот
- TO(layer) - активирует слой и деактивирует все остальные слои (кроме слоя по умолчанию). Эта функция особенная, потому что вместо того, чтобы просто добавлять/удалять один слой в ваш стек активных слоев, она полностью заменяет ваши текущие активные слои, уникально позволяя вам заменять более высокие слои более низкими. Это активируется при нажатии клавиши (как только клавиша нажата).
- TT (layer) - Нажмите-переключить слой. Если вы удерживаете клавишу нажатой, слой активируется, а затем деактивируется, когда вы отпускаете (например, `MO`). Если вы несколько раз коснетесь его, слой будет включен или выключен (например, `TG`). По умолчанию требуется 5 нажатий, но вы можете изменить это, определив `TAPPING_TOGGLE` — например, `#define TAPPING_TOGGLE 2`, чтобы переключаться только на два нажатия.


### Предостережения

В настоящее время аргумент слоя `LT()` ограничен слоями 0-15, а аргумент `kc` установлен базовым кодом клавиши, что означает, что вы не можете использовать коды клавиш, такие как `LCTL()`, `KC_TILD` или что-либо большее, чем 0xFF. Это связано с тем, что QMK использует 16-битные коды клавиш, из которых 4 бита используются для идентификатора функции и 4 бита для уровня, оставляя только 8 бит для кода клавиши.

По той же причине аргумент слоя `LM()` также ограничен слоями 0-15, а аргумент mod должен умещаться в пределах 5 бит. Как следствие, хотя левый и правый модификаторы поддерживаются `LM()`, невозможно смешивать и сопоставлять левые и правые модификаторы. Указание хотя бы одного правого модификатора в такой комбинации, как `MOD_RALT|MOD_LSFT`, приведет к преобразованию всех перечисленных модификаторов в их правосторонний аналог. Таким образом, использование вышеупомянутой мод-маски фактически отправит правый Alt + правый Shift. Обязательно используйте константы MOD_xxx вместо альтернативных способов указания модификаторов при определении ключа вашего слоя-мода.


|-------------------|---------------------------|---------------------------|-------------------|
| LM(1,KC_LSFT)		| LM(1,MOD_MASK_SHIFT)		| LM(1,MOD_BIT(KC_LSFT))	| LM(1,MOD_LSFT)	|
|-------------------|---------------------------|---------------------------|-------------------|
| ❌					| ❌							| ❌							| ✅					|
|-------------------|---------------------------|---------------------------|-------------------|


Расширение этого было бы сложно, в лучшем случае. Переход на 32-битный код клавиши решит многие из этих проблем, но удвоит объем пространства, используемого матрицей раскладки. И это тоже может вызвать проблемы. Если вам нужно применить модификаторы к нажатому ключевому коду, для этого можно использовать Tap Dance.

## Working with Layers

Care must be taken when switching layers, it’s possible to lock yourself into a layer with no way to deactivate that layer (without unplugging your keyboard.) We’ve created some guidelines to help users avoid the most common problems.

### Beginners

If you are just getting started with QMK you will want to keep everything simple. Follow these guidelines when setting up your layers:

- Setup layer 0 as your default, “base” layer. This is your normal typing layer, and could be whatever layout you want (qwerty, dvorak, colemak, etc.). It’s important to set this as the lowest layer since it will typically have most or all of the keyboard’s keys defined, so would block other layers from having any effect if it were above them (i.e., had a higher layer number).
- Arrange your layers in a “tree” layout, with layer 0 as the root. Do not try to enter the same layer from more than one other layer.
- In a layer’s keymap, only reference higher-numbered layers. Because layers are processed from the highest-numbered (topmost) active layer down, modifying the state of lower layers can be tricky and error-prone.


### Intermediate Users

Sometimes you need more than one base layer. For example, if you want to switch between QWERTY and Dvorak, switch between layouts for different countries, or switch your layout for different videogames. Your base layers should always be the lowest numbered layers. When you have multiple base layers you should always treat them as mutually exclusive. When one base layer is on the others are off.

### Advanced Users

Once you have a good feel for how layers work and what you can do, you can get more creative. The rules listed in the beginner section will help you be successful by avoiding some of the tricker details but they can be constraining, especially for ultra-compact keyboard users. Understanding how layers work will allow you to use them in more advanced ways.

Layers stack on top of each other in numerical order. When determining what a keypress does, QMK scans the layers from the top down, stopping when it reaches the first active layer that is not set to `KC_TRNS`. As a result if you activate a layer that is numerically lower than your current layer, and your current layer (or another layer that is active and higher than your target layer) has something other than `KC_TRNS`, that is the key that will be sent, not the key on the layer you just activated. This is the cause of most people’s “why doesn’t my layer get switched” problem.

Sometimes, you might want to switch between layers in a macro or as part of a tap dance routine. layer_on activates a layer, and layer_off deactivates it. More layer-related functions can be found in `action_layer.h`.

## Функции
There are a number of functions (and variables) related to how you can use or manipulate the layers.

| Function										| Description 																					|
|-----------------------------------------------|-------------------------------------------------------------------------------				|
| layer_state_set(layer_mask) 					| Directly sets the layer state (avoid unless you know what you are doing).						| 
| layer_clear()	 								| Clears all layers (turns them all off). 														| 
| layer_move(layer)								| Turns specified layer on, and all other layers off.											| 
| layer_on(layer)								| Turns specified layer on, leaves all other layers in existing state.							| 
| layer_off(layer)								| Turns specified layer off, leaves all other layers in existing state.| 
| layer_invert(layer)							| Interverts/toggles the state of the specified layer| 
| layer_or(layer_mask)							| Turns on layers based on matching bits between specifed layer and existing layer state.| 
| layer_and(layer_mask)							| Turns on layers based on matching enabled bits between specifed layer and existing layer state.| 
| layer_xor(layer_mask)							| Turns on layers based on non-matching bits between specifed layer and existing layer state.| 
| layer_debug(layer_mask)						| Prints out the current bit mask and highest active layer to debugger console.| 
| default_layer_set(layer_mask)					| Directly sets the default layer state (avoid unless you know what you are doing).| 
| default_layer_or(layer_mask)					| Turns on layers based on matching bits between specifed layer and existing default layer state.| 
| default_layer_and(layer_mask)					| Turns on layers based on matching enabled bits between specifed layer and existing default layer | state.| 
| default_layer_xor(layer_mask)					| Turns on layers based on non-matching bits between specifed layer and existing default layer state.| 
| default_layer_debug(layer_mask)				| Prints out the current bit mask and highest active default layer to debugger console.| 
| set_single_persistent_default_layer(layer)	| Sets the default layer and writes it to persistent memory (EEPROM).| 
| update_tri_layer(x, y, z)						| Checks if layers x and y are both on, and sets z based on that (on if both on, otherwise off).| 
| update_tri_layer_state(state, x, y, z)		| Does the same as update_tri_layer(x, y, z), but from layer_state_set_* functions.| 
| ----------------------------------------------| ----------------------------------------------------											|

In addition to the functions that you can call, there are a number of callback functions that get called every time the layer changes. This passes the layer state to the function, where it can be read or modified.

|-----------------------------------------------|------------								|
| Callback										| Description 								|
|-----------------------------------------------|------------								|
| layer_state_set_kb(layer_state_t state)		| Callback for layer functions, for keyboard.|
|layer_state_set_user(layer_state_t state)		| Callback for layer functions, for users.|
|default_layer_state_set_kb(layer_state_t state)	| Callback for default layer functions, for keyboard. Called on keyboard initialization.|
|default_layer_state_set_user(layer_state_t state)	| Callback for default layer functions, for users. Called on keyboard initialization.|
|-----------------------------------------------|------------|

For additional details on how you can use these callbacks, check out the [Layer Change Code ](https://docs.qmk.fm/#/custom_quantum_functions?id=layer-change-code) document.

It is also possible to check the state of a particular layer using the following functions and macros.

|-----|-----|------|
| Function							| Description												| Aliases |
| layer_state_is(layer)				| Checks if the specified layer is enabled globally.		| IS_LAYER_ON(layer), IS_LAYER_OFF(layer) |
| layer_state_cmp(state, layer)		| Checks state to see if the specified layer is enabled. Intended for use in layer callbacks.	| IS_LAYER_ON_STATE(state, layer), IS_LAYER_OFF_STATE(state, layer) |
|-----|-----|------|


## Layer Change Code

This runs code every time that the layers get changed. This can be useful for layer indication, or custom layer handling.

### Example layer_state_set_* Implementation

This example shows how to set the RGB Underglow lights based on the layer, using the Planck as an example.

```
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case _RAISE:
        rgblight_setrgb (0x00,  0x00, 0xFF);
        break;
    case _LOWER:
        rgblight_setrgb (0xFF,  0x00, 0x00);
        break;
    case _PLOVER:
        rgblight_setrgb (0x00,  0xFF, 0x00);
        break;
    case _ADJUST:
        rgblight_setrgb (0x7A,  0x00, 0xFF);
        break;
    default: //  for any other layers, or the default layer
        rgblight_setrgb (0x00,  0xFF, 0xFF);
        break;
    }
  return state;
}
```

Use the `IS_LAYER_ON_STATE(state, layer)` and `IS_LAYER_OFF_STATE(state, layer)` macros to check the status of a particular layer.

Outside of `layer_state_set_*` functions, you can use the `IS_LAYER_ON(layer)` and `IS_LAYER_OFF(layer)` macros to check global layer state.








## Setting up Visual Studio Code for QMK Development

Visual Studio Code (VS Code) is an open-source code editor that supports many different programming languages.

Using a full-featured editor such as VS Code provides many advantages over a plain text editor, such as:

- intelligent code completion
- convenient navigation in the code
- refactoring tools
- build automation (no need for the command-line)
- a graphical front end for GIT
- many other tools such as debugging, code formatting, showing call hierarchies etc.

The purpose of this page is to document how to set up VS Code for developing QMK Firmware.

This guide covers how to configure everything needed on Windows and Ubuntu 18.04

### Set up VS Code

Before starting, you will want to make sure that you have all of the build tools set up, and QMK Firmware cloned. Head to the [Newbs Getting Started Guide](https://docs.qmk.fm/#/newbs_getting_started) to get things set up, if you haven’t already.

#### Wimdows

##### Prerequisites

- Git for Windows (This link will prompt to save/run the installer)

1. Disable all of the options but Git LFS (Large File Support) and Check daily for Git for Windows updates.
2. Set the default editor to Use Visual Studio Code as Git's default editor
3. Select the Use Git from Git Bash only option, since that’s the option that you should use here.
4. For the Choosing HTTPS transport backend, either option should be fine.
5. Select the Checkout as-is, commit Unix-style line endings option. QMK Firmware uses Unix style commits.
6. For the extra options, leave the default options as is.

This software is needed for Git support in VS Code. It may be possible to not include this, but it is much simpler to just use this.

Git Credential Manager for Windows (Optional)

This software provides better support for Git by providing secure storage for git credentials, MFA and personal access token generation.

This isn’t strictly needed, but we would recommend it.

##### Installing VS Code

Head to VS Code and download the installer
Run the installer
This part is super simple. However, there is some configuration that we need to do to ensure things are configured correctly.

##### MSYS2 Setup

Now, we will set up the MSYS2 window to show up in VSCode as the integrated terminal. This has a number of advantages. Mostly, you can control+click on errors and jump to those files. This makes debugging much easier. It’s also nice, in that you don’t have to jump to another window.


#### Extensions

There are a number of extensions that you may want to install:

Git Extension Pack - This installs a bunch of Git related tools that may make using Git with QMK Firmware easier.
EditorConfig for VS Code - [Optional] - Helps to keep the code to the QMK Coding Conventions.
GitHub Markdown Preview - [Optional] - Makes the markdown preview in VS Code more like GitHub’s.
VS Live Share Extension Pack - [Optional] - This extension allows somebody else to access your workspace (or you to access somebody else’s workspace) and help out. This is great if you’re having issues and need some help from somebody.
Restart once you’ve installed any extensions


#### Configure VS Code for QMK

1. Click File > Open Folder
2. Open the QMK Firmware folder that you cloned from GitHub.
3. Click File > Save Workspace As...


##### Configuring VS Code

Using the standard compile_commands.json database, we can get VS code C/C++ extension to use the exact same includes and defines used for your keyboard and keymap.

Run qmk generate-compilation-database -kb <keyboard> -km <keymap> to generate the compile_commands.json.
Create .vscode/c_cpp_properties.json with the following content:

```
{
 "configurations": [
     {
         "name": "qmk",
         "compilerArgs": ["-mmcu=atmega32u4"],
         "compilerPath": "/usr/bin/avr-gcc",
         "cStandard": "gnu11",
         "cppStandard": "gnu++14",
         "compileCommands": "${workspaceFolder}/compile_commands.json",
         "intelliSenseMode": "linux-gcc-arm",
         "browse": {
             "path": [
                 "${workspaceFolder}"
             ],
             "limitSymbolsToIncludedHeaders": true,
             "databaseFilename": ""
         }
     }
 ],
 "version": 4
}
```








## =========
### История

Исторически QMK настраивался с помощью комбинации двух механизмов — rules.mk и config.h. Хотя это работало хорошо, когда в QMK было всего несколько клавиатур, мы выросли, чтобы охватить почти 1500 поддерживаемых клавиатур. Это экстраполирует до 6000 конфигурационных файлов только под клавиатурами/! Свободная форма этих файлов и уникальные шаблоны, которые люди использовали, чтобы избежать дублирования, усложнили текущее обслуживание, и большое количество наших клавиатур использует шаблоны, которые устарели и иногда трудны для понимания.

Мы также работаем над тем, чтобы донести мощь QMK до людей, которые не умеют работать с интерфейсом командной строки, а другие проекты, такие как VIA, работают над тем, чтобы сделать использование QMK таким же простым, как установка программы. Этим инструментам нужна информация о расположении клавиатуры или доступных выводах и функциях, чтобы пользователи могли в полной мере воспользоваться преимуществами QMK. Мы представили info.json как первый шаг к этому. QMK API — это попытка объединить эти 3 источника информации — config.h, rules.mk и info.json — в единый источник достоверной информации, который могут использовать инструменты конечного пользователя.

Теперь у нас есть поддержка генерации значений rules.mk и config.h из info.json, что позволяет нам иметь единый источник правды. Это позволит нам использовать автоматизированные инструменты для обслуживания клавиатур, экономя много времени и работ по обслуживанию, которые используются для конфигурации, управляемой данными.

## Обзор

На стороне C ничего не меняется. Когда вам нужно создать новое правило или определить, вы следуете тому же процессу:

Добавьте его в `docs/config_options.md`
Установите значение по умолчанию в соответствующем основном файле
Добавьте операторы `ifdef` по мере необходимости.
Затем вам нужно будет добавить поддержку вашей новой конфигурации в `info.json`. Основной процесс:

Добавьте его в схему в `data/schemas/keyboards.jsonschema`.
Добавить сопоставление в данные/карты (необязательно и не рекомендуется) Добавьте код для его извлечения/генерации в:
```
lib/python/qmk/info.py
lib/python/qmk/cli/генерировать/config_h.py
lib/python/qmk/cli/генерировать/rules_mk.py
```

## Добавление опции в info.json

В этом разделе описывается добавление поддержки значения config.h/rules.mk в info.json.

### Добавьте его в схему

QMK поддерживает файлы [jsonschema](https://json-schema.org/) в `data/schemas`. Значения, которые входят в файлы `info.json`, относящиеся к клавиатуре, хранятся в файле `keyboard.jsonschema`. Любое значение, которое вы хотите сделать доступным для редактирования конечными пользователями, должно быть здесь.

В некоторых случаях вы можете просто добавить новый ключ верхнего уровня. Вот несколько примеров: имя_клавиатуры, сопровождающий, процессор и URL-адрес. Это уместно, когда ваш вариант автономен и не связан напрямую с другими вариантами.

В других случаях вы должны сгруппировать похожие параметры вместе в объекте. Это особенно верно при добавлении поддержки функции. Вот несколько примеров, которым можно следовать: индикаторы, `matrix_pins` и `rgblight`. Если вы не знаете, как интегрировать свои новые опции, откройте вопрос или присоединитесь к `#cli` в Discord и начните там беседу.

### Добавить сопоставление

В большинстве случаев вы можете добавить простое сопоставление. Они хранятся в виде файлов JSON в `data/mappings/info_config.hjson` и `data/mappings/info_rules.hjson`, а также управляют сопоставлением для `config.h` и `rules.mk`соответственно. Каждое сопоставление определяется переменной `config.h` или `rules.mk`, а значение представляет собой хэш со следующими ключами:

`info_key`		: (обязательно) местоположение в файле `info.json` для этого значения. См. ниже.
`value_type`	: (необязательно) Необработанный по умолчанию. Формат значения этой переменной. См. ниже.
`to_json`		: (необязательно) по умолчанию `true`. Установите значение `false`, чтобы исключить это сопоставление из `info.json`.
`to_c`			: (необязательно) по умолчанию `true`. Установите значение `false`, чтобы исключить это сопоставление из `config.h`.
`warn_duplicate`: (необязательно) Истинно по умолчанию. Установите значение `false`, чтобы отключить предупреждение, когда значение существует в обоих местах.

### Информационный ключ

Мы используем точечную нотацию JSON для адресации переменных в `info.json`. Например, для доступа к `info_json["rgblight"]["split_count"]` я бы указал `rgblight.split_count`. Это позволяет обращаться к глубоко вложенным ключам с помощью простой строки.

Под капотом мы используем Dotty Dict, вы можете обратиться к этой документации, чтобы узнать, как эти строки преобразуются в доступ к объектам.

### Типы значений

По умолчанию мы рассматриваем все значения как «сырые» данные без кавычек. Если ваше значение более сложное, вы можете использовать один из этих типов для интеллектуального анализа данных:
- массив	: массив строк, разделенных запятыми
- array.int : массив целых чисел, разделенных запятыми.
- int 		: целое число
- hex 		: число в шестнадцатеричном формате
- list 		: массив строк, разделенный пробелами
- mapping 	: хэш пар ключ/значение
- str 		: строковый литерал в кавычках


### Добавьте код, чтобы извлечь его

Большинство случаев использования могут быть решены с помощью файлов отображения, описанных выше. Если вы не можете, вы можете вместо этого написать код для извлечения значений конфигурации.

Всякий раз, когда QMK генерирует полный файл `info.json`, он извлекает информацию из `config.h` и `rules.mk`. Вам нужно будет добавить код для вашего нового значения конфигурации в `lib/python/qmk/info.py`. Обычно это означает добавление новой функции `_extract_<feature>()` и последующий вызов вашей функции либо в `_extract_config_h(), либо в _extract_rules_mk()`.

Если вы не знаете, как редактировать этот файл, или вам не нравится Python, откройте вопрос или присоединитесь к #cli в Discord, и кто-нибудь может помочь вам с этой частью.


### Добавьте код для его генерации

Последняя часть головоломки — предоставление вашей новой опции системе сборки. Это делается путем создания двух файлов:
```
.build/obj_<клавиатура>/src/info_config.h
.build/obj_<клавиатура>/src/rules.mk
```
Эти два файла генерируются кодом здесь:
```
lib/python/qmk/cli/генерировать/config_h.py
lib/python/qmk/cli/генерировать/rules_mk.py
```
Для значений `config.h` вам нужно написать функцию для ваших правил и вызвать эту функцию в `generate_config_h()`.

Если у вас есть новый ключ `info.json` верхнего уровня для `rules.mk`, вы можете просто добавить свои ключи в `info_to_rules` в верхней части `lib/python/qmk/cli/generate/rules_mk.py`. В противном случае вам нужно будет создать новый блок `if` для вашей функции в `generate_rules_mk()`.


