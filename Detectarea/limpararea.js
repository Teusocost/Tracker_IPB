// Seu arquivo de texto
const texto = `
<coordinates>
-6.769515857838626,41.79885658643625,0 -6.770318368810831,41.79823927828824,0 -6.770763521829978,41.79694155188922,0 -6.769811573283917,41.79556119064422,0 -6.768390859009022,41.79459540148369,0 -6.767403798164903,41.79518237984895,0 -6.765892598773458,41.79598064615573,0 -6.76369058450773,41.79707699702205,0 -6.764319206821443,41.79840850057142,0 -6.765933857670367,41.79904462708071,0 -6.768145778727074,41.79879874181806,0 -6.769515857838626,41.79885658643625,0 
</coordinates>
</LinearRing>
`;

// Encontre o trecho entre as tags <coordinates>
const coordenadas = texto.match(/<coordinates>(.*?)<\/coordinates>/s);

if (coordenadas) {
  const dados = coordenadas[1].trim().split(' ');

  // Processa os dados e cria pares [lat, lon]
  const paresLatLon = dados.map((coord) => {
    const partes = coord.split(',');
    // Remove os "0" à direita das vírgulas
    const lon = parseFloat(partes[0]);
    const lat = parseFloat(partes[1]);
    return [lat, lon];
  });

  console.log(paresLatLon);
} else {
  console.log('Trecho de coordenadas não encontrado no arquivo.');
}
