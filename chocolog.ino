#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"

// --- Configurações de Pinos ---
#define DHTPIN 2
#define DHTTYPE DHT11
#define LDR_PIN A0
#define BUZZER_PIN 8
#define LED_VERDE_PIN 10
#define LED_VERM_PIN 9
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
#define ADDR_TIMEZONE   1002
#define ADDR_TEMP_UNIT  1003 // NOVO: Endereço da unidade de temperatura

// --- Triggers (Limites Exigidos pelo Projeto) ---
float trigger_t_min = 15.0;
float trigger_t_max = 22.0;
float trigger_u_min = 45.0;
float trigger_u_max = 60.0;
int trigger_l_max = 50;

// --- Variáveis Globais ---
int idioma = 0; // 0 = PT, 1 = EN, 2 = ES
int fuso = -3;  // UTC
int unidadeTemp = 0; // 0 = Celsius, 1 = Fahrenheit // NOVO

unsigned long tempoAnteriorLCD = 0;
bool mostraRelogio = true;

bool alarmeAtivo = false;
char telaAlarmeLinha1[24];
char telaAlarmeLinha2[24];
unsigned long tempoBuzzer = 0;
bool estadoBuzzer = false;

// ==========================================
// DESENHO CUSTOMIZADO: ANIMAÇÃO DE CHOCOLATE
// ==========================================
byte chocNormal[8] = { B11111, B10001, B10101, B10001, B10101, B10001, B10001, B11111 };
byte chocDestaque[8] = { B11111, B11111, B11011, B11111, B11011, B11111, B11111, B11111 };
byte chocQuebraDireita[8] = { B11110, B10010, B10111, B10010, B10110, B10011, B10010, B11110 };
byte chocQuebraEsquerda[8] = { B01111, B01001, B11101, B01001, B01101, B11001, B01001, B01111 };

// ==========================================
// DICIONÁRIO MULTI-IDIOMA (0 = PT, 1 = EN, 2 = ES)
// ==========================================
const char* txtIdioma[] = {"Portugues", "English", "Espanol"};
const char* txtMenuFuso[] = {"Fuso Horario UTC", "Timezone UTC", "Zona Horaria UTC"};
const char* txtMenuTemp[] = {"Medida de Temp.", "Temp. Unit", "Medida Temp."}; // NOVO
const char* txtUnidade[] = {"Celsius (C)", "Fahrenheit(F)"}; // NOVO
const char* txtIniciando[] = {"Iniciando Log...", "Starting Log...", "Iniciando Log..."};

const char* txtData[] = {"DATA: ", "DATE: ", "FECHA:"};
const char* txtHora[] = {"HORA: ", "TIME: ", "HORA: "};
const char* txtUmidTag[] = {"U:", "H:", "H:"};     
const char* txtLuzTag[]  = {"L:", "L:", "L:"};

const char* txtAlerta[] = {"ALERTA! ", "ALERT!  ", "ALERTA! "};
const char* txtAlarmeOff[] = {"Alarme Desligado", "Alarm Turned Off", "Alarma Apagada  "};
const char* txtSemLogs[] = {"Nenhum Erro", "No Errors Logged", "Sin Errores"};
const char* txtSaindo[] = {"Saindo...", "Exiting...", "Saliendo..."};

const char* txtConfigAno[] = {"Ano (2024+):", "Year (2024+):", "Ano (2024+):"};
const char* txtConfigMes[] = {"Mes (1-12):", "Month (1-12):", "Mes (1-12):"};
const char* txtConfigDia[] = {"Dia (1-31):", "Day (1-31):", "Dia (1-31):"};
const char* txtConfigHora[] = {"Hora (0-23):", "Hour (0-23):", "Hora (0-23):"};
const char* txtConfigMin[] = {"Minuto (0-59):", "Minute (0-59):", "Minuto (0-59):"};


// ==========================================
// FUNÇÕES DA ANIMAÇÃO DE BOOT
// ==========================================

void delayEChecaBotao(int ms, bool &flagMenu) {
    unsigned long inicio = millis();
    while (millis() - inicio < ms) {
        if (digitalRead(BTN_OK) == LOW) {
            flagMenu = true;
        }
        delay(10);
    }
}

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

void exibirAnimacaoChocolate(bool &forcarMenu) {
    lcd.createChar(0, chocNormal);
    lcd.createChar(1, chocDestaque);
    lcd.createChar(2, chocQuebraDireita);
    lcd.createChar(3, chocQuebraEsquerda);

    const byte inicio = 5;

    desenharChocolateInteiro(inicio, false); delayEChecaBotao(500, forcarMenu);
    desenharChocolateInteiro(inicio, true);  delayEChecaBotao(220, forcarMenu);
    desenharChocolateInteiro(inicio, false); delayEChecaBotao(160, forcarMenu);
    desenharChocolateInteiro(inicio, true);  delayEChecaBotao(220, forcarMenu);

    desenharChocolateSeparado(inicio, inicio + 3); delayEChecaBotao(180, forcarMenu);
    desenharChocolateSeparado(inicio, inicio + 4); delayEChecaBotao(180, forcarMenu);
    desenharChocolateSeparado(inicio, inicio + 5); delayEChecaBotao(180, forcarMenu);
    desenharChocolateSeparado(inicio, inicio + 6); delayEChecaBotao(450, forcarMenu);

    desenharChocolateSeparado(inicio, inicio + 5); delayEChecaBotao(110, forcarMenu);
    desenharChocolateSeparado(inicio, inicio + 6); delayEChecaBotao(300, forcarMenu);
}


void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    pinMode(LED_VERM_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    
    dht.begin();
    lcd.init();
    lcd.backlight();
    
    if (!RTC.begin()) {
        lcd.print("Erro no RTC!");
        while (1); 
    }

    // --- RECUPERAÇÃO DO PONTEIRO DA EEPROM ---
    currentAddress = 0;
    for (int i = startAddress; i < endAddress; i += recordSize) {
        uint32_t tempoSalvo;
        EEPROM.get(i, tempoSalvo);
        if (tempoSalvo == 0xFFFFFFFF || tempoSalvo == 0) {
            currentAddress = i; // Encontra o espaço livre correto
            break;
        }
    }

    bool forcarMenu = false;
    
    // 1. Toca a animação do chocolate
    exibirAnimacaoChocolate(forcarMenu);

    // 2. Tela "ChocoLog" separada
    lcd.clear();
    lcd.setCursor(1, 0); 
    lcd.write((uint8_t)0); 
    lcd.print(" ChocoLog ");
    lcd.write((uint8_t)0); 
    delayEChecaBotao(1500, forcarMenu); 

    // 3. Tela de aviso do Menu separada
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hold OK = Menu  ");
    delayEChecaBotao(1500, forcarMenu); 

    byte magicByte = EEPROM.read(ADDR_MAGIC_BYTE);
    
    // Verifica se precisa abrir o menu de configuração
    if (magicByte != 0xAA || forcarMenu || !RTC.isrunning()) {
        executarMenuConfiguracao();
    } else {
        idioma = EEPROM.read(ADDR_LANGUAGE);
        fuso = EEPROM.read(ADDR_TIMEZONE) - 12; 
        unidadeTemp = EEPROM.read(ADDR_TEMP_UNIT); // Carrega unidade da memória
        if (unidadeTemp > 1) unidadeTemp = 0; // Proteção contra lixo de memória
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(txtIniciando[idioma]);
    delay(1500);
}

void loop() {
    // ---------------------------------------------------------
    // ESTADO 1: ALARME ATIVADO
    // ---------------------------------------------------------
    if (alarmeAtivo) {
        digitalWrite(LED_VERDE_PIN, LOW);
        digitalWrite(LED_VERM_PIN, HIGH); 

        lcd.setCursor(0, 0); lcd.print(telaAlarmeLinha1);
        lcd.setCursor(0, 1); lcd.print(telaAlarmeLinha2);

        tone(BUZZER_PIN, 3000); 

        if (digitalRead(BTN_OK) == LOW) {
            alarmeAtivo = false;
            noTone(BUZZER_PIN); 
            digitalWrite(LED_VERM_PIN, LOW);
            
            lcd.clear();
            lcd.print(txtAlarmeOff[idioma]);
            
            while(digitalRead(BTN_OK) == LOW) delay(10); 
            delay(1500);
            lcd.clear();
        }
        return; 
    }

    // ---------------------------------------------------------
    // ESTADO 2: MONITORAMENTO NORMAL
    // ---------------------------------------------------------
    digitalWrite(LED_VERM_PIN, LOW); 

    if (digitalRead(BTN_OK) == LOW) {
        delay(300); 
        exibirLogsLCD(); 
        lcd.clear(); 
    }

    DateTime tempoReal = RTC.now();
    uint32_t unixAjustado = tempoReal.unixtime() + (fuso * 3600);
    DateTime now = DateTime(unixAjustado); 
    
    float temperaturaAtual = dht.readTemperature(); // Medição nativa e lógica sempre em C
    float umidadeAtual = dht.readHumidity();
    int leituraLDR = analogRead(LDR_PIN);
    int luminosidadeAtual = map(leituraLDR, 400, 870, 100, 0); 

    if (now.minute() != lastLoggedMinute) {
        lastLoggedMinute = now.minute();
        
        // Avaliação de triggers mantida intocada (baseada em Celsius)
        if (temperaturaAtual < trigger_t_min || temperaturaAtual > trigger_t_max || 
            umidadeAtual < trigger_u_min || umidadeAtual > trigger_u_max ||
            luminosidadeAtual > trigger_l_max) {
            
            salvarLogEEPROM(tempoReal.unixtime(), temperaturaAtual, umidadeAtual, luminosidadeAtual);
            
            // --- CÁLCULO EXCLUSIVO PARA O DISPLAY DE ALARME ---
            int tempAlarme = (int)temperaturaAtual;
            if (unidadeTemp == 1) tempAlarme = (int)((temperaturaAtual * 1.8) + 32.0);
            char charTemp = (unidadeTemp == 0) ? 'C' : 'F';
            
            sprintf(telaAlarmeLinha1, "%s%02d:%02d", txtAlerta[idioma], now.hour(), now.minute());
            // Atualizado para incluir C ou F dinamicamente colado no número (ex: T:25C)
            sprintf(telaAlarmeLinha2, "T:%d%c %s%d %s%d", 
                    tempAlarme, charTemp, txtUmidTag[idioma], (int)umidadeAtual, txtLuzTag[idioma], luminosidadeAtual);
            
            alarmeAtivo = true;
            lcd.clear();
            return; 
        }
    }

    if (millis() - tempoAnteriorLCD >= 3000) {
        tempoAnteriorLCD = millis();
        mostraRelogio = !mostraRelogio; 
        lcd.clear();
        
        digitalWrite(LED_VERDE_PIN, mostraRelogio ? HIGH : LOW);

        if (mostraRelogio) {
            char bufferData[24];
            char bufferHora[24];
            sprintf(bufferData, "%s%02d/%02d/%04d", txtData[idioma], now.day(), now.month(), now.year());
            sprintf(bufferHora, "%s%02d:%02d:%02d", txtHora[idioma], now.hour(), now.minute(), now.second());
            lcd.setCursor(0, 0); lcd.print(bufferData);
            lcd.setCursor(0, 1); lcd.print(bufferHora);
        } else {
            lcd.setCursor(0, 0);
            
            // --- CÁLCULO EXCLUSIVO PARA O DISPLAY NORMAL ---
            int tempExibir = (int)temperaturaAtual;
            if (unidadeTemp == 1) tempExibir = (int)((temperaturaAtual * 1.8) + 32.0);
            char charTemp = (unidadeTemp == 0) ? 'C' : 'F';
            
            lcd.print("T:"); lcd.print(tempExibir); lcd.print(charTemp); lcd.print(" ");
            
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
    while (digitalRead(BTN_OK) == LOW) {
        delay(10);
    }

    bool confirmado = false;
    while (!confirmado) {
        lcd.setCursor(0, 0); lcd.print("Language/Idioma:");
        lcd.setCursor(0, 1); lcd.print("> "); lcd.print(txtIdioma[idioma]); lcd.print("        ");
        
        if (digitalRead(BTN_UP) == LOW) { idioma++; if(idioma > 2) idioma = 0; delay(300); }
        if (digitalRead(BTN_DOWN) == LOW) { idioma--; if(idioma < 0) idioma = 2; delay(300); }
        if (digitalRead(BTN_OK) == LOW) { confirmado = true; delay(300); }
    }

    confirmado = false;
    lcd.clear();
    while (!confirmado) {
        lcd.setCursor(0, 0); lcd.print(txtMenuFuso[idioma]);
        lcd.setCursor(0, 1); 
        lcd.print("> "); if (fuso > 0) lcd.print("+"); lcd.print(fuso); lcd.print("   ");

        if (digitalRead(BTN_UP) == LOW) { fuso++; if(fuso>12) fuso=12; delay(200); }
        if (digitalRead(BTN_DOWN) == LOW) { fuso--; if(fuso<-12) fuso=-12; delay(200); }
        if (digitalRead(BTN_OK) == LOW) { confirmado = true; delay(300); }
    }

    // --- NOVO: MENU DE UNIDADE DE TEMPERATURA ---
    confirmado = false;
    lcd.clear();
    while (!confirmado) {
        lcd.setCursor(0, 0); lcd.print(txtMenuTemp[idioma]);
        lcd.setCursor(0, 1); 
        lcd.print("> "); lcd.print(txtUnidade[unidadeTemp]); lcd.print("  ");

        if (digitalRead(BTN_UP) == LOW) { unidadeTemp++; if(unidadeTemp>1) unidadeTemp=0; delay(200); }
        if (digitalRead(BTN_DOWN) == LOW) { unidadeTemp--; if(unidadeTemp<0) unidadeTemp=1; delay(200); }
        if (digitalRead(BTN_OK) == LOW) { confirmado = true; delay(300); }
    }

    int ano = ajustarValorDisplay(txtConfigAno[idioma], 2024, 2024, 2050);
    int mes = ajustarValorDisplay(txtConfigMes[idioma], 1, 1, 12);
    int dia = ajustarValorDisplay(txtConfigDia[idioma], 1, 1, 31);
    int hora = ajustarValorDisplay(txtConfigHora[idioma], 12, 0, 23);
    int min = ajustarValorDisplay(txtConfigMin[idioma], 0, 0, 59);

    EEPROM.write(ADDR_LANGUAGE, idioma);
    EEPROM.write(ADDR_TIMEZONE, fuso + 12); 
    EEPROM.write(ADDR_TEMP_UNIT, unidadeTemp); // Salva unidade na memória
    EEPROM.write(ADDR_MAGIC_BYTE, 0xAA);

    DateTime horaLocal(ano, mes, dia, hora, min, 0);
    uint32_t horaUTC = horaLocal.unixtime() - (fuso * 3600);
    RTC.adjust(DateTime(horaUTC));

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
            valor++; if (valor > maxVal) valor = minVal; 
            while(digitalRead(BTN_UP) == LOW) delay(10); 
            delay(50); 
        }
        if (digitalRead(BTN_DOWN) == LOW) {
            valor--; if (valor < minVal) valor = maxVal; 
            while(digitalRead(BTN_DOWN) == LOW) delay(10); 
            delay(50); 
        }
        if (digitalRead(BTN_OK) == LOW) {
            confirmado = true; 
            while(digitalRead(BTN_OK) == LOW) delay(10); 
            delay(50); 
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
        
        uint32_t tAjustado = t + (fuso * 3600);
        DateTime dt = DateTime(tAjustado);
        
        char linha1[24]; char linha2[24]; 
        
        // --- CÁLCULO EXCLUSIVO PARA O DISPLAY DE HISTÓRICO ---
        int tempExibir = tempInt / 100;
        if (unidadeTemp == 1) tempExibir = (int)((tempExibir * 1.8) + 32.0);
        char charTemp = (unidadeTemp == 0) ? 'C' : 'F';
        
        sprintf(linha1, "L:%02d %02d/%02d %02d:%02d", (indexAtual+1), dt.day(), dt.month(), dt.hour(), dt.minute());
        // Atualizado para mostrar o C ou F acompanhando o número
        sprintf(linha2, "T:%d%c %s%d %s%d", tempExibir, charTemp, txtUmidTag[idioma], (humiInt/100), txtLuzTag[idioma], luz);

        lcd.setCursor(0, 0); lcd.print(linha1);
        lcd.setCursor(0, 1); lcd.print(linha2);

        if (digitalRead(BTN_UP) == LOW) { 
            indexAtual++; if (indexAtual >= totalLogs) indexAtual = 0; 
            lcd.clear(); 
            while(digitalRead(BTN_UP) == LOW) delay(10); 
            delay(50); 
        }
        if (digitalRead(BTN_DOWN) == LOW) { 
            indexAtual--; if (indexAtual < 0) indexAtual = totalLogs - 1; 
            lcd.clear(); 
            while(digitalRead(BTN_DOWN) == LOW) delay(10); 
            delay(50); 
        }
        if (digitalRead(BTN_OK) == LOW) { 
            sair = true; 
            while(digitalRead(BTN_OK) == LOW) delay(10); 
            delay(50); 
        }
    }

    lcd.clear(); lcd.setCursor(0,0); lcd.print(txtSaindo[idioma]);
    delay(1000);
}
