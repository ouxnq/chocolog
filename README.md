# ChocoLog - Data Logger de Transporte 🍫

<img width="1361" height="956" alt="chocologWowki" src="https://github.com/user-attachments/assets/5a318b2c-33dd-479b-ba9b-2818d45c53d6" />

Um sistema autônomo baseado em Arduino para monitoramento ambiental durante o transporte de chocolates no continente Americano. O projeto registra desvios críticos de temperatura, umidade e luminosidade, garantindo o controle de qualidade da carga por meio de armazenamento em memória não volátil (EEPROM) e alertas visuais/sonoros imediatos por meio de LEDs e Buzzer.

## 🛠️ Hardware Necessário

| Componente | Pino no Arduino | Observações |
| :--- | :--- | :--- |
| **Microcontrolador** | - | Arduino Uno R3 (ATmega 328P) |
| **Sensor DHT11** | Digital 2 | Leitura de Temperatura e Umidade |
| **Fotoresistência (LDR)** | Analógico A0 | Requer resistor de 10kΩ (Divisor de tensão) |
| **Módulo RTC DS1307** | A4 (SDA) e A5 (SCL) | Relógio em Tempo Real (Comunicação I2C) |
| **Display LCD 16x2 I2C** | A4 (SDA) e A5 (SCL) | Interface de usuário (IHM) |
| **Buzzer** | Digital 8 | Alerta sonoro |
| **LED Verde** | Digital 9 | Indicador visual de funcionamento normal |
| **LED Vermelho** | Digital 10 | Indicador visual de alerta crítico |
| **Botão CIMA (+)** | Digital 5 | Navegação de menus (Usa INPUT_PULLUP) |
| **Botão BAIXO (-)** | Digital 6 | Navegação de menus (Usa INPUT_PULLUP) |
| **Botão OK / Sair** | Digital 7 | Confirmação e reset de alarme (Usa INPUT_PULLUP) |

*(Nota: O hardware deve ser alimentado por uma Bateria de 9V. O encapsulamento físico do projeto em gabinete isola a placa, expondo apenas a IHM e os indicadores).*

## 🔬 Especificações Técnicas e Precisão dos Sensores

Para garantir a confiabilidade dos dados transportados, os componentes operam sob as seguintes capacidades técnicas:
* **Temperatura (DHT11):** Medição em Graus Celsius (°C). Faixa de leitura: 0 a 50 °C. Precisão de ±2.0 °C.
* **Umidade (DHT11):** Medição em Percentual Relativo (%). Faixa de leitura: 20 a 90 %. Precisão de ±5.0 %.
* **Luminosidade (LDR):** Medição em Percentual Estimado (0 a 100%). Conversão analógica baseada no ADC de 10 bits do Arduino utilizando um divisor de tensão de 10kΩ.

## 📖 Manual de Instruções e Operação

### 1. Primeira Inicialização (Setup)
Ao ligar o equipamento pela primeira vez (ou caso a bateria do relógio acabe), o sistema apresentará uma animação de inicialização e entrará automaticamente no menu de configuração da IHM.
* Utilize os botões **CIMA** e **BAIXO** para navegar entre as opções.
* Pressione **OK** para confirmar.
* Você deverá configurar, em ordem: **Idioma** (Português, Inglês ou Espanhol), **Fuso Horário**, **Ano**, **Mês**, **Dia**, **Hora** e **Minuto**.

> **Nota para Testes:** Para forçar a abertura do menu de configuração em um equipamento já configurado, mantenha o botão **OK** pressionado durante a inicialização (enquanto a tela exibe a mensagem "Segure OK = Menu").

### 2. Monitoramento Normal
Durante a viagem, o sistema opera de forma autônoma. O **LED Verde** acenderá e apagará a cada 3 segundos, servindo como um "heartbeat" para indicar que o sistema está em operação saudável. O display acompanhará o LED alternando entre duas telas:
* **Tela de Relógio:** Exibe a Data e Hora atuais.
* **Tela de Sensores:** Exibe os dados em tempo real de Temperatura (T), Umidade (H/U) e Luminosidade (Luz/Lgt).

### 3. Disparo de Alarme (Gatilhos)
O alarme disparará imediatamente se os sensores detectarem valores fora das rigorosas margens de segurança para conservação do chocolate:
* **Temperatura:** 15 < T < 25 ºC (Abaixo de 15°C ou acima de 25°C).
* **Umidade:** 30% < U < 50% (Abaixo de 30% ou acima de 50%).
* **Luminosidade:** 0 < L < 30% (Acima de 30%, indicando exposição excessiva à luz ou possível quebra da embalagem).

**O que acontece durante o alarme:**
1. O evento é imediatamente gravado na memória não volátil (EEPROM) do ATmega 328P.
2. O display trava exibindo uma tela de alerta e os valores exatos (timestamp e sensores) do momento da infração.
3. O monitoramento contínuo é pausado.
4. O **LED Verde se apaga**.
5. O **LED Vermelho e o Buzzer começam a piscar/apitar ininterruptamente**.

* **Para silenciar:** O operador logístico deve visualizar os dados na tela e pressionar o botão **OK** para reconhecer o erro. Os alertas visuais e sonoros serão desligados, e o sistema voltará ao monitoramento normal.

### 4. Leitura do Histórico (Logs)
Para auditar o controle de qualidade da carga ao final da viagem, leia o histórico utilizando a própria IHM. Durante o monitoramento normal, pressione o botão **OK**. O display exibirá os erros salvos cronologicamente a partir da EEPROM. A primeira linha mostra o número do registro e o timestamp. A segunda linha exibe as condições climáticas gravadas no momento da infração. Use os botões de navegação para percorrer os logs, e o botão OK para sair.

## ⚙️ Modificando os Parâmetros (Para Desenvolvedores)
Se precisar alterar as margens de tolerância do chocolate, modifique as seguintes variáveis no escopo global do arquivo `.ino`:

```cpp
float trigger_t_min = 20.0;
float trigger_t_max = 30.0;
float trigger_u_min = 30.0;
float trigger_u_max = 60.0;
int trigger_l_max = 20;
