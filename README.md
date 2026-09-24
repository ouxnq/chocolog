## 🛠️ Hardware Necessário

| Componente | Pino no Arduino | Observações |
| :--- | :--- | :--- |
| **Sensor DHT22** | Digital 2 | Temperatura e Umidade |
| **Fotoresistência (LDR)** | Analógico A0 | Requer resistor de 10kΩ (Divisor de tensão) |
| **Módulo RTC DS1307** | A4 (SDA) e A5 (SCL) | Relógio em Tempo Real (I2C) |
| **Display LCD 16x2 I2C** | A4 (SDA) e A5 (SCL) | Interface visual |
| **Buzzer** | Digital 8 | Alarme sonoro |
| **Botão CIMA (+)** | Digital 5 | Navegação (Usa INPUT_PULLUP) |
| **Botão BAIXO (-)** | Digital 6 | Navegação (Usa INPUT_PULLUP) |
| **Botão OK / Sair** | Digital 7 | Confirmação e reset de alarme (Usa INPUT_PULLUP) |
