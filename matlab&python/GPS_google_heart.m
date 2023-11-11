% Coordinate dei punti (solo longitudine e latitudine)
coordinates = [
    14.13564008290164,42.45638933853132;
    14.13570114422611,42.45642258583789;
    14.13573004544705,42.4564927138641;
    14.13570187482711,42.45656869702832;
    14.13566858160563,42.45659002344726;
    14.13562833637383,42.45660216105944;
    14.13554025625185,42.45661625411417;
    14.13542624933452,42.45663587242306;
    14.1353389921849,42.45662436418704;
    14.1353046230462,42.45660033370964;
    14.13528144108153,42.45657074451086;
    14.13526908406548,42.45653758082745;
    14.13526617568013,42.45651170990243;
    14.13528413715374,42.45646900996181;
    14.13534286733688,42.45642829130698;
    14.13544269016338,42.4564043867596;
    14.13564008290164,42.45638933853132;

];

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
    x = center(1) + radius * cos(theta);
    y = center(2) + radius * sin(theta);
    plot(x, y, 'r', 'LineWidth', 1.5);

    % Impostazioni del grafico
    xlabel('Longitudine');
    ylabel('Latitudine');
    title('Tracciato Google Heart');
    axis equal;
end



