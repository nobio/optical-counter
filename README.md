# Docker build
Irgendwie funktiontioniert derzeit der Build in Github nicht, zumindest ist das entstehende Image nicht auf dem Raspberry PI ausführbar. Bauen bitte lokal

`docker build --platform linux/arm/v7 --no-cache -t nobio/optical-counter:latest .`

`docker push nobio/optical-counter:latest`

# Auswertung
Entweder mit irgendeiner KI oder tesseract.js (https://github.com/naptha/tesseract.js/blob/master/docs/examples.md)