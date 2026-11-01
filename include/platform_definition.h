#ifndef PLATFORM_DEFINITION_H
#define PLATFORM_DEFINITION_H

// --- определение архитектуры и чипа ---
#if defined(ESP8266)
    #define ARCH_ESP8266
#else
    #define ESP32_FAMILY // общий флаг для всего семейства ESP32
    #if defined(CONFIG_IDF_TARGET_ESP32C3)
        #define TARGET_ESP32C3
    #else
        #define TARGET_ESP32
    #endif
#endif

#endif