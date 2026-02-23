/*
Código gerado pelo Chat GPT e refinado conforme as necessidades do projeto em 20/02/2026
https://github.com/marcelovictorio @MarceloVictorio YouTube
*/

/*
Atenção! A biblioteca ESP32Servo.h precisa ser instalada separadamente assim como as demais. (Ref. John K. Bennett)
Diferente da biblioteca Servo.h padrão (que já vem na IDE para placas Arduino comuns), 
o ESP32 exige essa versão específica para gerenciar corretamente os timers e os canais PWM internos do chip. 
*/
/*
🛰️ RADAR ULTRASSÔNICO IoT – RADAR CINEMATOGRÁFICO
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ===== CONFIGURAÇÃO DE PINES =====
#define SERVO_PIN   13
#define TRIG_PIN    5
#define ECHO_PIN    18
#define BUZZER_PIN  4

const char* ssid = "ESP32_Radar_Web";
WebServer server(80);
Servo myServo;

int currentAngle = 0;
int currentDistance = 0;
int step = 1;

// Função para medir distância
int medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH, 25000); // Timeout reduzido para estabilidade
  int d = duracao * 0.034 / 2;
  if (d == 0 || d > 40) return 40; 
  return d;
}

// Função para o som do Buzzer (Efeito Militar)
void tocarBuzzer(int distancia) {
  if (distancia > 0 && distancia < 30) {
    // Se houver objeto perto, o bip é disparado
    digitalWrite(BUZZER_PIN, HIGH);
    delay(15); // Bip curto e agudo
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// Interface HTML com visual moderno (Neon Radar)
void paginaPrincipal() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Radar Militar ESP32-C3</title>
  <style>
    body { background: #0a0a0a; color: #00ff41; font-family: 'Courier New', monospace; text-align: center; }
    canvas { background: #000; border: 1px solid #004400; border-radius: 50% 50% 0 0; box-shadow: 0 0 20px #003300; }
    .status { color: #ff0000; font-weight: bold; text-transform: uppercase; margin-bottom: 10px; }
  </style>
</head>
<body>
  <div style="margin-top:20px;">
    <div class="status" id="alert">-- SCANNING --</div>
    <canvas id="radarCanvas"></canvas>
    <h3>ANG: <span id="ang">0</span>° | DIST: <span id="dist">0</span>cm</h3>
  </div>

  <script>
    const canvas = document.getElementById("radarCanvas");
    const ctx = canvas.getContext("2d");
    canvas.width = 400; canvas.height = 230;
    const cX = 200, cY = 220, r = 200;

    function draw(angle, dist) {
      ctx.fillStyle = "rgba(0, 10, 0, 0.05)"; 
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      
      // Grades do Radar
      ctx.strokeStyle = "#003300";
      for(let i=1; i<=4; i++) {
        ctx.beginPath(); ctx.arc(cX, cY, (r/4)*i, Math.PI, 2*Math.PI); ctx.stroke();
      }

      let rad = (angle * Math.PI) / 180;
      let x = cX - r * Math.cos(rad);
      let y = cY - r * Math.sin(rad);
      
      // Linha de Varredura
      ctx.strokeStyle = "#00ff41";
      ctx.lineWidth = 3;
      ctx.beginPath(); ctx.moveTo(cX, cY); ctx.lineTo(x, y); ctx.stroke();

      // Alvo detectado
      if(dist < 30) {
        document.getElementById("alert").innerHTML = "!!! OBJETO DETECTADO !!!";
        let pX = cX - (dist * (r/40)) * Math.cos(rad);
        let pY = cY - (dist * (r/40)) * Math.sin(rad);
        ctx.fillStyle = "#ff0000";
        ctx.beginPath(); ctx.arc(pX, pY, 6, 0, 2*Math.PI); ctx.fill();
      } else {
        document.getElementById("alert").innerHTML = "-- SCANNING --";
      }
    }

    setInterval(() => {
      fetch("/data").then(r => r.json()).then(d => {
        document.getElementById("ang").innerHTML = d.a;
        document.getElementById("dist").innerHTML = d.d;
        draw(d.a, d.d);
      });
    }, 40);
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void enviarDados() {
  String json = "{\"a\":" + String(currentAngle) + ",\"d\":" + String(currentDistance) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);

  WiFi.softAP(ssid);
  server.on("/", paginaPrincipal);
  server.on("/data", enviarDados);
  server.begin();
}

void loop() {
  server.handleClient();
  
  static unsigned long t = 0;
  if (millis() - t > 35) { // Controla a velocidade da varredura
    currentAngle += step;
    if (currentAngle >= 170 || currentAngle <= 10) step *= -1;
    
    myServo.write(currentAngle);
    currentDistance = medirDistancia();
    
    // Toca o buzzer apenas se detectar algo no ângulo atual
    tocarBuzzer(currentDistance);
    
    t = millis();
  }
}