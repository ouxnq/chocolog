#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"

// --- Configurações de Pinos ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define LDR_PIN A0
#define BUZZER_PIN 8
#define BTN_UP 5
#define BTN_DOWN 6
#define BTN_OK 7   

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;

// --- Configurações da EEPROM ---
const int maxRecords = 100;
const int recordSize = 10;
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;
int lastLoggedMinute = -1;

#define ADDR_MAGIC_BYTE 1000
#define ADDR_LANGUAGE   1001

// --- Triggers (Limites Aceitáveis) ---
float trigger_t_min = 20.0;
float trigger_t_max = 30.0;
float trigger_u_min = 30.0;
float trigger_u_max = 60.0;
int trigger_l_max = 20;

// --- Variáveis Globais ---
int idioma = 0; // 0 = PT, 1 = EN, 2 = ES
unsigned long tempoAnteriorLCD = 0;
bool mostraRelogio = true;

bool alarmeAtivo = false;
char telaAlarmeLinha1[17];
char telaAlarmeLinha2[17];
unsigned long tempoBuzzer = 0;
bool estadoBuzzer = false;

// ==========================================
// DICIONÁRIO MULTI-IDIOMA (0 = PT, 1 = EN, 2 = ES)
// ==========================================
const char* txtIdioma[] = {"Portugues", "English", "Espanol"};
const char* txtIniciando[] = {"Iniciando Log...", "Starting Log...", "Iniciando Log..."};

const char* txtData[] = {"DATA: ", "DATE: ", "FECHA:"};
const char* txtHora[] = {"HORA: ", "TIME: ", "HORA: "};
const char* txtUmidTag[] = {"U:", "H:", "H:"};     
const char* txtLuzTag[]  = {"Luz:", "Lgt:", "Luz:"}; 

const char* txtAlerta[] = {"ALERTA! ", "ALERT!  ", "ALERTA! "};
const char* txtAlarmeOff[] = {"Alarme Desligado", "Alarm Turned Off", "Alarma Apagada  "};
const char* txtSemLogs[] = {"Nenhum Erro", "No Errors Logged", "Sin Errores"};
const char* txtSaindo[] = {"Saindo...", "Exiting...", "Saliendo..."};

// Textos do Menu de Data/Hora
const char* txtConfigAno[] = {"Ano (2024+):", "Year (2024+):", "Ano (2024+):"};
const char* txtConfigMes[] = {"Mes (1-12):", "Month (1-12):", "Mes (1-12):"};
const char* txtConfigDia[] = {"Dia (1-31):", "Day (1-31):", "Dia (1-31):"};
const char* txtConfigHora[] = {"Hora (0-23):", "Hour (0-23):", "Hora (0-23):"};
const char* txtConfigMin[] = {"Minuto (0-59):", "Minute (0-59):", "Minuto (0-59):"};
const char* txtAperteOK[] = {"Hold OK = Config"};

// ==========================================
// ANIMACAO INICIAL - BARRA DE CHOCOLATE
// ==========================================
byte chocNormal[8] = {
    B11111, B10001, B10101, B10001, B10101, B10001, B10001, B11111
};
byte chocDestaque[8] = {
    B11111, B11111, B11011, B11111, B11011, B11111, B11111, B11111
};
byte chocQuebraDireita[8] = {
    B11110, B10010, B10111, B10010, B10110, B10011, B10010, B11110
};
byte chocQuebraEsquerda[8] = {
    B01111, B01001, B11101, B01001, B01101, B11001, B01001, B01111
};

void desenharChocolateInteiro(byte colunaInicial, bool destacarUltimaBarra) {
    lcd.clear();
    lcd.setCursor(colunaInicial, 0);
    lcd.write((uint8_t)0); lcd.write((uint8_t)0); lcd.write((uint8_t)0);
    lcd.write((uint8_t)(destacarUltimaBarra ? 1 : 0));
    lcd.setCursor(colunaInicial, 1);
    lcd.write((uint8_t)0); lcd.write((uint8_t)0); lcd.write((uint8_t)0);
    lcd.write((uint8_t)(destacarUltimaBarra ? 1 : 0));
}

void desenharChocolateSeparado(byte colunaInicial, byte colunaPedaco) {
    lcd.clear();
    lcd.setCursor(colunaInicial, 0);
    lcd.write((uint8_t)0); lcd.write((uint8_t)0); lcd.write((uint8_t)2);
    lcd.setCursor(colunaInicial, 1);
    lcd.write((uint8_t)0); lcd.write((uint8_t)0); lcd.write((uint8_t)2);
    
    lcd.setCursor(colunaPedaco, 0); lcd.write((uint8_t)3);
    lcd.setCursor(colunaPedaco, 1); lcd.write((uint8_t)3);
}

void exibirAnimacaoChocolate() {
    lcd.createChar(0, chocNormal);
    lcd.createChar(1, chocDestaque);
    lcd.createChar(2, chocQuebraDireita);
    lcd.createChar(3, chocQuebraEsquerda);
    const byte inicio = 6; 

    desenharChocolateInteiro(inicio, false); delay(500);
    desenharChocolateInteiro(inicio, true); delay(220);
    desenharChocolateInteiro(inicio, false); delay(160);
    desenharChocolateInteiro(inicio, true); delay(220);

    desenharChocolateSeparado(inicio, inicio + 3); delay(180);
    desenharChocolateSeparado(inicio, inicio + 4); delay(180);
    desenharChocolateSeparado(inicio, inicio + 5); delay(180);
    desenharChocolateSeparado(inicio, inicio + 6); delay(450);

    desenharChocolateSeparado(inicio, inicio + 5); delay(110);
    desenharChocolateSeparado(inicio, inicio + 6); delay(300);
    lcd.clear();
}

// ==========================================
// INICIALIZAÇÃO
// ==========================================
void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    
    dht.begin();
    lcd.init();
    lcd.backlight();
    
    // Executa a animação de boot antes de checar as configurações
    exibirAnimacaoChocolate();
    
    if (!RTC.begin()) {
        lcd.print("Erro no RTC!");
        while (1); 
    }

    lcd.setCursor(0, 0);
    lcd.print(txtAperteOK[0]); 
    delay(2500); 

    byte magicByte = EEPROM.read(ADDR_MAGIC_BYTE);
    bool forcarMenu = (digitalRead(BTN_OK) == LOW); 
    
    if (magicByte != 0xAA || forcarMenu || !RTC.isrunning()) {
        executarMenuConfiguracao();
    } else {
        idioma = EEPROM.read(ADDR_LANGUAGE);
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(txtIniciando[idioma]);
    delay(1500);
}

void loop() {
    // ---------------------------------------------------------
    // ESTADO 1: ALARME ATIVADO (SISTEMA TRAVADO)
    // ---------------------------------------------------------
    if (alarmeAtivo) {
        lcd.setCursor(0, 0); lcd.print(telaAlarmeLinha1);
        lcd.setCursor(0, 1); lcd.print(telaAlarmeLinha2);

        if (millis() - tempoBuzzer >= 500) {
            tempoBuzzer = millis();
            estadoBuzzer = !estadoBuzzer;
            digitalWrite(BUZZER_PIN, estadoBuzzer ? HIGH : LOW);
        }

        if (digitalRead(BTN_OK) == LOW) {
            alarmeAtivo = false;
            digitalWrite(BUZZER_PIN, LOW);
            lcd.clear();
            lcd.print(txtAlarmeOff[idioma]);
            delay(1500);
            lcd.clear();
        }
        return; 
    }

    // ---------------------------------------------------------
    // ESTADO 2: MONITORAMENTO NORMAL
    // ---------------------------------------------------------
    if (digitalRead(BTN_OK) == LOW) {
        delay(300); 
        exibirLogsLCD(); 
        lcd.clear(); 
    }

    DateTime now = RTC.now(); 
    
    float temperaturaAtual = dht.readTemperature();
    float umidadeAtual = dht.readHumidity();
    int leituraLDR = analogRead(LDR_PIN);
    
    // CORREÇÃO: Escala invertida para acompanhar o hardware do LDR (100 a 0)
    int luminosidadeAtual = map(leituraLDR, 0, 1023, 100, 0); 

    if (now.minute() != lastLoggedMinute) {
        lastLoggedMinute = now.minute();
        
        if (temperaturaAtual < trigger_t_min || temperaturaAtual > trigger_t_max || 
            umidadeAtual < trigger_u_min || umidadeAtual > trigger_u_max ||
            luminosidadeAtual > trigger_l_max) {
            
            salvarLogEEPROM(now.unixtime(), temperaturaAtual, umidadeAtual, luminosidadeAtual);
            
            sprintf(telaAlarmeLinha1, "%s%02d:%02d", txtAlerta[idioma], now.hour(), now.minute());
            sprintf(telaAlarmeLinha2, "T:%d %s%d %s%d", 
                    (int)temperaturaAtual, txtUmidTag[idioma], (int)umidadeAtual, txtLuzTag[idioma], luminosidadeAtual);
            
            alarmeAtivo = true;
            lcd.clear();
            return; 
        }
    }

    if (millis() - tempoAnteriorLCD >= 3000) {
        tempoAnteriorLCD = millis();
        mostraRelogio = !mostraRelogio; 
        lcd.clear();
        
        if (mostraRelogio) {
            char bufferData[17];
            char bufferHora[17];
            sprintf(bufferData, "%s%02d/%02d/%04d", txtData[idioma], now.day(), now.month(), now.year());
            sprintf(bufferHora, "%s%02d:%02d:%02d", txtHora[idioma], now.hour(), now.minute(), now.second());
            lcd.setCursor(0, 0); lcd.print(bufferData);
            lcd.setCursor(0, 1); lcd.print(bufferHora);
        } else {
            lcd.setCursor(0, 0);
            lcd.print("T:"); lcd.print((int)temperaturaAtual); lcd.print("C");
            lcd.setCursor(8, 0);
            lcd.print(txtUmidTag[idioma]); lcd.print((int)umidadeAtual); lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print(txtLuzTag[idioma]); lcd.print(" "); lcd.print(luminosidadeAtual); lcd.print("%");
        }
    }
}

// ==========================================
// FUNÇÕES AUXILIARES E MENUS
// ==========================================

void executarMenuConfiguracao() {
    bool confirmado = false;
    while (!confirmado) {
        lcd.setCursor(0, 0); lcd.print("Language/Idioma:");
        lcd.setCursor(0, 1); lcd.print("> "); lcd.print(txtIdioma[idioma]); lcd.print("        ");
        
        if (digitalRead(BTN_UP) == LOW) { idioma++; if(idioma > 2) idioma = 0; delay(300); }
        if (digitalRead(BTN_DOWN) == LOW) { idioma--; if(idioma < 0) idioma = 2; delay(300); }
        if (digitalRead(BTN_OK) == LOW) { confirmado = true; delay(300); }
    }

    int ano = ajustarValorDisplay(txtConfigAno[idioma], 2024, 2024, 2050);
    int mes = ajustarValorDisplay(txtConfigMes[idioma], 1, 1, 12);
    int dia = ajustarValorDisplay(txtConfigDia[idioma], 1, 1, 31);
    int hora = ajustarValorDisplay(txtConfigHora[idioma], 12, 0, 23);
    int min = ajustarValorDisplay(txtConfigMin[idioma], 0, 0, 59);

    EEPROM.write(ADDR_LANGUAGE, idioma);
    EEPROM.write(ADDR_MAGIC_BYTE, 0xAA);
    RTC.adjust(DateTime(ano, mes, dia, hora, min, 0));
    lcd.clear();
}

int ajustarValorDisplay(const char* titulo, int valorInicial, int minVal, int maxVal) {
    int valor = valorInicial;
    bool confirmado = false;
    lcd.clear();
    
    while (!confirmado) {
        lcd.setCursor(0, 0); lcd.print(titulo);
        lcd.setCursor(0, 1); lcd.print("> "); lcd.print(valor); lcd.print("    ");

        if (digitalRead(BTN_UP) == LOW) {
            valor++; if (valor > maxVal) valor = minVal; delay(200);
        }
        if (digitalRead(BTN_DOWN) == LOW) {
            valor--; if (valor < minVal) valor = maxVal; delay(200);
        }
        if (digitalRead(BTN_OK) == LOW) {
            confirmado = true; delay(300);
        }
    }
    return valor;
}

void salvarLogEEPROM(uint32_t tempo, float temp, float umid, int luz) {
    int tempInt = (int)(temp * 100);
    int humiInt = (int)(umid * 100);
    
    EEPROM.put(currentAddress, tempo);
    EEPROM.put(currentAddress + 4, tempInt);
    EEPROM.put(currentAddress + 6, humiInt);
    EEPROM.put(currentAddress + 8, luz); 
    
    currentAddress += recordSize;
    if (currentAddress >= endAddress) currentAddress = 0;
}

void exibirLogsLCD() {
    int totalLogs = 0;
    int enderecosValidos[maxRecords]; 

    for (int address = startAddress; address < endAddress; address += recordSize) {
        uint32_t timeStamp;
        EEPROM.get(address, timeStamp);
        if (timeStamp != 0xFFFFFFFF && timeStamp != 0) { 
            enderecosValidos[totalLogs] = address;
            totalLogs++;
        }
    }

    if (totalLogs == 0) {
        lcd.clear(); lcd.setCursor(0, 0); lcd.print(txtSemLogs[idioma]);
        delay(2000); return;
    }

    int indexAtual = 0; 
    bool sair = false;

    while (!sair) {
        uint32_t t; int tempInt, humiInt, luz;
        EEPROM.get(enderecosValidos[indexAtual], t);
        EEPROM.get(enderecosValidos[indexAtual] + 4, tempInt);
        EEPROM.get(enderecosValidos[indexAtual] + 6, humiInt);
        EEPROM.get(enderecosValidos[indexAtual] + 8, luz);
        
        DateTime dt = DateTime(t);
        
        char linha1[17]; char linha2[17];
        sprintf(linha1, "%02d/%02d %02d/%02d %02dh", (indexAtual+1), totalLogs, dt.day(), dt.month(), dt.hour());
        sprintf(linha2, "T:%d %s%d %s%d", (tempInt/100), txtUmidTag[idioma], (humiInt/100), txtLuzTag[idioma], luz);

        lcd.setCursor(0, 0); lcd.print(linha1);
        lcd.setCursor(0, 1); lcd.print(linha2);

        if (digitalRead(BTN_UP) == LOW) { indexAtual++; if (indexAtual >= totalLogs) indexAtual = 0; delay(300); }
        if (digitalRead(BTN_DOWN) == LOW) { indexAtual--; if (indexAtual < 0) indexAtual = totalLogs - 1; delay(300); }
        if (digitalRead(BTN_OK) == LOW) { sair = true; delay(300); }
    }

    lcd.clear(); lcd.setCursor(0,0); lcd.print(txtSaindo[idioma]);
    delay(1000);
}