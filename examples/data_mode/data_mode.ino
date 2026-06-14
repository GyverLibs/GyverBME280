/*
    Пример настроек получаеых данных
    О режимах:
    MODE_DATA_DEFAULT - По умолчанию, стандартная реализация, при ошибки получения даннх с датичика возвращает 0 
    MODE_DATA_LAST_VALID - при ошибки получения даннх с датичика возвращает последне полученное значение
    MODE_DATA_INTERPOLATED_LINE - линейная интрополяция данных с датчика
    MODE_DATA_INTERPOLATED_GAUS - интрополяция по гаусу данных с датчика
*/

#include <GyverBME280.h>                                // Подключение библиотеки
GyverBME280 bme;                                        // Создание обьекта bme

void setup() {
    Serial.begin(9600);                         // Запуск последовательного порта
    bme.setMode(MODE_DATA_LAST_VALID);                  // устанавливаем мод без 0 при ошибках
    bme.initInterpolateData();                          // инциализируем данные по умолчанию чтобы убрать мусор
    bme.begin();                                        // Если доп. настройки не нужны  - инициализируем датчик
}

void loop() {
    Serial.print("Temperature: ");
    Serial.print(bme.readTemperature());        // Выводим темперутуру в [*C]
    Serial.println(" *C");

    Serial.print("Humidity: ");
    Serial.print(bme.readHumidity());           // Выводим влажность в [%]
    Serial.println(" %");

    float pressure = bme.readPressure();        // Читаем давление в [Па]
    Serial.print("Pressure: ");
    Serial.print(pressure / 100.0F);            // Выводим давление в [гПа]
    Serial.print(" hPa , ");
    Serial.print(pressureToMmHg(pressure));     // Выводим давление в [мм рт. столба]
    Serial.println(" mm Hg");
    Serial.print("Altitide: ");
    Serial.print(pressureToAltitude(pressure)); // Выводим высоту в [м над ур. моря]
    Serial.println(" m");
    Serial.println("");
    delay(1000);
}