% Imposta il nome del file .csv da caricare
file_name = 'coordinate100ms.csv';

% Carica i dati dal file .csv
coordinates = csvread(file_name);

coordinates = coordinates(:, [2, 1]);

% Chiamata alla funzione per disegnare il percorso chiuso
plotPath(coordinates);

function plotPath(coordinates)
    % Assumiamo che la prima coordinata sia quella iniziale
    coordinates = [coordinates; coordinates(1,:)];

    hold on;

    % Disegna le rette che connettono le coordinate
    plot(coordinates(:,1), coordinates(:,2), 'b-o');
    
    % Segna il punto di partenza con un pallino rosso pieno
    plot(coordinates(1,1), coordinates(1,2), 'ro', 'MarkerFaceColor', 'r');
    
    % Crea una circonferenza attorno al punto di partenza
    radius = 0.00003; 
    center = coordinates(1,:);
    theta = linspace(0, 2*pi, 100);
    x = center(1) + radius * sin(theta);
    y = center(2) + radius * cos(theta);
    plot(x, y, 'r', 'LineWidth', 1.5);
    

    % Impostazioni del grafico
    xlabel('Longitudine');
    ylabel('Latitudine');
    title('Tracciato GPS');
    axis equal;
end
