import serial

# Imposta le informazioni della porta seriale (devi modificare queste informazioni con i dettagli della tua porta seriale)
ser_port = 'COM3'  # Modifica la porta seriale in base al tuo sistema operativo (es. 'COM3' su Windows)
baud_rate = 115200

# Crea un oggetto seriale
ser = serial.Serial(ser_port, baud_rate)

output_file = 'coordinate100ms-2.csv'  # Specifica il nome del file di output

try:
    with open(output_file, 'w') as file:
        while True:
            # Leggi una riga dalla seriale (terminatore di riga '\n')
            line = ser.readline().decode('utf-8').strip()

            # Salva le informazioni nel file solo se il formato è corretto (latitudine, longitudine)
            if ',' in line:
                lat, lon = line.split(',')
                file.write(f"{lat}, {lon}\n")

            # Stampa il dato ricevuto (opzionale)
            print(line)

except KeyboardInterrupt:
    # Interrompi il programma se viene premuto Ctrl+C
    print("Programma interrotto manualmente.")

finally:
    # Chiudi la connessione seriale in caso di eccezione o uscita normale dal loop
    ser.close()
