* Создание сертификата (пароль юзать лучше changeit, имя - localhost)  
```keytool -genkeypair -alias localhost -keyalg RSA -keysize 2048 -keystore anton.jks -validity 3650```
* Генерация файла сертификата (.crt)  
```keytool -exportcert -keystore anton.jks -alias [alias] -file [cert_file]```
* Импорт в кейстор  
```keytool -importkeystore -srckeystore anton.jks -destkeystore keystore.jks```
* Импорт в трастстор (паяра)  
```keytool -importcert -trustcacerts -destkeystore cacerts.jks -file danaserver.crt -alias danaserver```