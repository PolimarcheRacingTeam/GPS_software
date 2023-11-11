% Apri i due file in MATLAB
f1 = fopen('GPS_google_heart.m');
f2 = fopen('GPS_tracking.m');


% Crea un nuovo pannello di figure
figure;

% Aggiungi il primo grafico al nuovo pannello
subplot(211)
plot(f1);

% Aggiungi il secondo grafico al nuovo pannello
subplot(212)
plot(f2);

% Posiziona i due grafici nel pannello in modo che siano uno sotto l'altro
set(gcf, 'Position', [100 100 600 400])