void initUpgrade() {
  // Добавляем функцию Update для перезаписи прошивки по Wi-Fi при 1М(256K SPIFFS) и выше
  httpUpdater.setup(&HTTP);
  HTTP.on("/upgrade", webUpgrade);                // запустить обнавление
  modulesReg("upgrade");
}
// ----------------------- Обновление с сайта
void webUpgrade() {
  String refresh = "<html><head><meta http-equiv=\"refresh\" content=\"40;/\">Update module...</head></html>";
  HTTP.send(200, "text/html", refresh);
  String spiffsData = HTTP.arg("spiffs");
  if (spiffsData != "") {
    SPIFFS.format();
    spiffsData = spiffsData.substring(spiffsData.lastIndexOf("/") + 1); // выделяем имя файла
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(HTTP.arg("spiffs"));
switch(ret) {
            case HTTP_UPDATE_FAILED:
            configSetup = jsonWrite(configSetup, "spiffsData", "HTTP_UPDATE_FAILED");
            statistics();
            break;
            case HTTP_UPDATE_NO_UPDATES:
            configSetup = jsonWrite(configSetup, "spiffsData", "HTTP_UPDATE_NO_UPDATES");
            statistics();
                break;
            case HTTP_UPDATE_OK:
            configSetup = jsonWrite(configSetup, "spiffsData", "HTTP_UPDATE_OK");
            statistics();
                break;
        }
    writeFile("timer.save.json", jsonTimer);
    writeFile("scenary.save.txt", Scenary);
    configSetup = jsonWrite(configSetup, "spiffsData", spiffsData);
    saveConfigSetup ();
  }
  String buildData = HTTP.arg("build");
  if (buildData != "") {
    buildData = buildData.substring(buildData.lastIndexOf("/") + 1); // выделяем имя файла
     configSetup = jsonWrite(configSetup, "buildData", buildData);
    saveConfigSetup ();
    ESPhttpUpdate.rebootOnUpdate(true);
    t_httpUpdate_return jet = ESPhttpUpdate.update(HTTP.arg("build"));
  }
}


