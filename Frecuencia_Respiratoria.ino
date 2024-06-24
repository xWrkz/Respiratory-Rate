#include <LiquidCrystal.h>

#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

// Definición de pines adicionales y constantes
#define LCD_CONTRAST_PIN A1    // Pin del potenciómetro para controlar el contraste
#define SOUND_SENSOR_PIN A0    // Pin analógico conectado al sensor de sonido KY-037
#define LED_PIN 13             // Pin digital para controlar el LED

// Tamaño del LCD (16x2)
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Creación de objeto para el LCD
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Variables para medir la frecuencia respiratoria y el sonido
unsigned long lastTime = 0;
unsigned long respTime = 0;
unsigned int respCount = 0;
float respRate = 0;
int threshold = 0; // Umbral para detectar respiraciones
int soundValue = 0; // Valor del sensor de sonido
unsigned long respInterval = 10000; // Intervalo para calcular la frecuencia respiratoria (10 segundos)

// Variables para el filtro de promedio móvil
const int numReadings = 10;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

void setup() {
  // Inicializar el LCD
  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  // Inicializar el pin del sensor de sonido y el LED
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Inicializar el contraste del LCD con el potenciómetro
  analogWrite(LCD_CONTRAST_PIN, 128); // Ajusta el valor según sea necesario
  
  // Esperar un momento para que se estabilice el LCD
  delay(1000);
  
  // Limpiar la pantalla LCD y mostrar un mensaje inicial
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");
  delay(2000); // Esperar 2 segundos

  // Calibrar el umbral
  calibrateThreshold();

  // Inicializar el arreglo de lecturas
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
  Serial.begin(9600);
}

void loop() {
  // Leer el valor del sensor de sonido
  soundValue = analogRead(SOUND_SENSOR_PIN);

  // Aplicar el filtro de promedio móvil
  total = total - readings[readIndex];
  readings[readIndex] = soundValue;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  average = total / numReadings;

  // Imprimir el valor del sensor para monitoreo en el monitor serie
  Serial.print("Valor del sensor de sonido: ");
  Serial.println(average);
  
  // Controlar el LED según el valor promedio del sensor de sonido
  if (average > threshold) {
    digitalWrite(LED_PIN, HIGH); // Encender el LED si el sonido supera el umbral
  } else {
    digitalWrite(LED_PIN, LOW); // Apagar el LED si el sonido está por debajo del umbral
  }

  // Detectar un aumento en el valor del sensor como una respiración
  if (average > threshold && millis() - respTime > 200) {
    respCount++;
    respTime = millis(); // Reiniciar el tiempo de debounce
  }
  
  // Calcular la frecuencia respiratoria cada 10 segundos
  if (millis() - lastTime >= respInterval) {
    lastTime = millis(); // Reiniciar el tiempo de cálculo

    // Calcular la frecuencia respiratoria en respiraciones por minuto
    respRate = (float)respCount * 6.0; // Convertir respiraciones por 10 segundos a respiraciones por minuto

    // Reiniciar el conteo de respiraciones
    respCount = 0;

    // Determinar el tipo de apnea según la frecuencia respiratoria y el sonido
   String freqText;
   if (respRate < 12) {
     freqText = "Tipo: Bradipnea";
   } else if (respRate > 20) {
     freqText = "Tipo: Taquipnea";
   } else {
     freqText = "Tipo: Normal";
   }
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Freq: ");
   lcd.print(respRate);
   lcd.print(" RPM");
   lcd.setCursor(0, 1);
   lcd.print(freqText);
   delay(1000);
 }
}

void calibrateThreshold() {
  int sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += analogRead(SOUND_SENSOR_PIN);
    delay(10);
  }
  threshold = sum / 100 + 50; // Calibrar umbral basado en el promedio de las lecturas más un margen
  Serial.print("Umbral calibrado: ");
  Serial.println(threshold);
}
