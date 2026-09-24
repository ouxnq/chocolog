\# ChocoLog - Data Logger de Transporte 🍫



Um sistema autônomo baseado em Arduino para monitoramento ambiental durante o transporte de chocolates. O projeto registra desvios críticos de temperatura, umidade e luminosidade, garantindo o controle de qualidade da carga por meio de armazenamento em memória não volátil (EEPROM) e alertas visuais/sonoros.



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



---



\## 📖 Manual de Instruções e Operação



\### 1. Primeira Inicialização (Setup)

Ao ligar o equipamento pela primeira vez (ou caso a bateria do relógio acabe), o sistema apresentará uma animação de inicialização e entrará automaticamente no menu de configuração.

\* Utilize os botões \*\*CIMA\*\* e \*\*BAIXO\*\* para navegar entre as opções.

\* Pressione \*\*OK\*\* para confirmar.

\* Você deverá configurar, em ordem: \*\*Idioma\*\* (Português, Inglês ou Espanhol), \*\*Ano\*\*, \*\*Mês\*\*, \*\*Dia\*\*, \*\*Hora\*\* e \*\*Minuto\*\*.



> \*\*Nota para Testes:\*\* Para forçar a abertura do menu de configuração em um equipamento já configurado, mantenha o botão \*\*OK\*\* pressionado durante a inicialização (enquanto a tela exibe a mensagem "Segure OK = Menu").



\### 2. Monitoramento Normal

Durante a viagem, o sistema opera de forma autônoma e silenciosa. O display alternará a cada 3 segundos entre duas telas:

\* \*\*Tela de Relógio:\*\* Exibe a Data e Hora atuais.

\* \*\*Tela de Sensores:\*\* Exibe os dados em tempo real de Temperatura (T), Umidade (H/U) e Luminosidade (Luz/Lgt).



\### 3. Disparo de Alarme

O chocolate é sensível a variações climáticas. O alarme disparará se os sensores detectarem valores fora das seguintes margens de segurança:

\* \*\*Temperatura:\*\* Abaixo de 20°C ou acima de 30°C.

\* \*\*Umidade:\*\* Abaixo de 30% ou acima de 60%.

\* \*\*Luminosidade:\*\* Acima de 20% (indicando exposição à luz, possível quebra de embalagem ou abertura do contêiner).



\*\*O que acontece durante o alarme:\*\*

1\. O evento é imediatamente gravado na memória EEPROM.

2\. O display trava exibindo uma tela de alerta e os valores exatos do momento da infração.

3\. O Buzzer começa a apitar.

4\. O monitoramento é pausado.

\* \*\*Para silenciar:\*\* O operador deve visualizar os dados na tela e pressionar o botão \*\*OK\*\* para reconhecer o erro. O alarme será desligado e o monitoramento normal será retomado.



\### 4. Leitura do Histórico (Logs)

Para auditar a carga ao final da viagem, você pode ler o histórico de duas maneiras:



\*\*Pelo próprio aparelho (Offline):\*\*

1\. Durante o monitoramento normal, pressione o botão \*\*OK\*\*.

2\. O display exibirá os erros salvos. A primeira linha mostra o número do registro (ex: `01/05`) e o timestamp (Dia, Mês e Hora). A segunda linha exibe as condições climáticas gravadas no momento do erro.

3\. Use os botões \*\*CIMA\*\* e \*\*BAIXO\*\* para navegar pelos logs antigos.

4\. Pressione \*\*OK\*\* novamente para sair.



\*\*Pelo Computador (Modo Manutenção):\*\*

1\. Conecte o Arduino ao computador via cabo USB.

2\. Abra o Monitor Serial (9600 baud).

3\. Envie a letra `L` (maiúscula ou minúscula).

4\. O sistema imprimirá uma tabela formatada com todos os logs, facilitando a cópia para planilhas.



---



\## ⚙️ Modificando os Parâmetros (Para Desenvolvedores)

Se precisar alterar as margens de tolerância do chocolate, modifique as seguintes variáveis no escopo global do arquivo `.ino`:

```cpp

float trigger\_t\_min = 20.0;

float trigger\_t\_max = 30.0;

float trigger\_u\_min = 30.0;

float trigger\_u\_max = 60.0;

int trigger\_l\_max = 20;

