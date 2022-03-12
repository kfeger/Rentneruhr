
void handleRoot() {
  String Message;
  char Compile[32];
  sprintf(Compile, "%s %s", __DATE__, __TIME__);
  strftime(TimeChar, 80, "%a, %e.%m.%G, %H:%M:%S %Z", &tm);
  Message = "<html><body><h1>";
  Message += "Projekt " + BaseFile + "</h1><br>";
  Message += "<h2>Hostname = " + String(Hostname);
  Message += "<br> Datum und Uhrzeit: " + String(TimeChar);
  Message += "<br>Stepper-Position = " + String(CurrentPosition);
  Message += "</h2>";
  Message += "<br>Kompiliert am " + String(__DATE__) + " um " + String(__TIME__);
  Message += "<br><b>&copy; ottO 2022</b>";
  Message += "</body></html>";
  //server.send(200, "text/html", "<html><body><h2>Stepper-MQTT\r\n</h2></body></html>");
  server.send(200, "text/html", Message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}
