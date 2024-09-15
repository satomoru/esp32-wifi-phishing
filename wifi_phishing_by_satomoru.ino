#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "@SATOMORU_YT";  // Wi-Fi nomi
const char* password = "12345678";  // Wi-Fi paroli
const char* adminPassword = "admin123";  // Admin sahifasi uchun parol

WebServer server(80);
DNSServer dnsServer;

struct User {
  String phoneNumber;
  String ip;
  String mac;
  String connectTime;
};

User users[10];  // Foydalanuvchilar ma'lumotlarini saqlash uchun array
int userCount = 0;  // Foydalanuvchi sonini kuzatish
bool isAuthenticated = false;  // Admin uchun parol tekshiruvi

// Foydalanuvchilarni qabul qilish sahifasi
void handleRoot() {
  String html = "<!DOCTYPE html><html><body>";
  html += "<h1>wifiga ulanish uchun telfon raqamingizni kiriting</h1>";
  html += "<form action=\"/submit\" method=\"POST\">";
  html += "Telefon raqami: <input type=\"text\" name=\"phone\" pattern=\"\\+998[0-9]{9}\" required>";
  html += "<input type=\"submit\" value=\"Yuborish\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  if (server.method() == HTTP_POST) {
    String phoneNumber = server.arg("phone");
    if (userCount < 10) {  
      users[userCount].phoneNumber = phoneNumber;
      users[userCount].ip = server.client().remoteIP().toString();  
      users[userCount].mac = WiFi.softAPmacAddress();  
      users[userCount].connectTime = String(millis() / 1000) + " seconds";
      userCount++;
    }
    server.send(200, "text/html", "<h1>Raqamingiz qabul qilindi! Endi internetga ulanasiz.</h1>");
  }
}

void handleAdminLogin() {
  String html = "<!DOCTYPE html><html><body>";
  html += "<h1>Admin paneliga kirish uchun parolni kiriting</h1>";
  html += "<form action=\"/admin\" method=\"POST\">";
  html += "Parol: <input type=\"password\" name=\"password\" required>";
  html += "<input type=\"submit\" value=\"Kirish\">";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleAdmin() {
  if (server.method() == HTTP_POST) {
    String passwordInput = server.arg("password");
    if (passwordInput == adminPassword) {
      isAuthenticated = true;
    } else {
      isAuthenticated = false;
      server.send(200, "text/html", "<h1>Noto'g'ri parol!</h1>");
      return;
    }
  }

  if (isAuthenticated) {
    String html = "<!DOCTYPE html><html><body>";
    html += "<h1>Wi-Fi foydalanuvchilari ro'yxati</h1><ul>";
    for (int i = 0; i < userCount; i++) {
      html += "<li>" + users[i].phoneNumber + " - IP: " + users[i].ip + " - MAC: " + users[i].mac + " - Ulanish vaqti: " + users[i].connectTime + "</li>";
    }
    html += "</ul>";
    
    html += "<h2>Foydalanuvchini o'chirish</h2>";
    html += "<form action=\"/delete\" method=\"POST\">";
    html += "Telefon raqami: <input type=\"text\" name=\"phone\" required>";
    html += "<input type=\"submit\" value=\"O'chirish\">";
    html += "</form>";
    
    html += "<h2>Ma'lumotlarni yuklash</h2>";
    html += "<a href=\"/download\" download><button>Yuklab olish</button></a>";
    
    html += "</body></html>";
    server.send(200, "text/html", html);
  } else {
    handleAdminLogin(); 
  }
}

void handleDeleteUser() {
  String phoneNumber = server.arg("phone");
  for (int i = 0; i < userCount; i++) {
    if (users[i].phoneNumber == phoneNumber) {
      for (int j = i; j < userCount - 1; j++) {
        users[j] = users[j + 1];  
      }
      userCount--;
      server.send(200, "text/html", "<h1>Foydalanuvchi o'chirildi!</h1>");
      return;
    }
  }
  server.send(200, "text/html", "<h1>Foydalanuvchi topilmadi!</h1>");
}


void handleDownload() {
  String csv = "Phone,IP,MAC,ConnectTime\n";
  for (int i = 0; i < userCount; i++) {
    csv += users[i].phoneNumber + "," + users[i].ip + "," + users[i].mac + "," + users[i].connectTime + "\n";
  }
  server.send(200, "text/csv", csv);
}

void handleNotFound() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);


  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  dnsServer.start(53, "*", IP);


  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.on("/admin", handleAdmin);
  server.on("/delete", handleDeleteUser); 
  server.on("/download", handleDownload); 
  server.onNotFound(handleNotFound);
  
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();  
  server.handleClient();  
}
