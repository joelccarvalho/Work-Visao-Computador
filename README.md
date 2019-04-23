Este trabalho tem como objetivo o desenvolvimento de um programa em C que permita a automática identificação e quantificação de fichas de poker numa imagem. Neste sentido, o programa deverá efetuar a leitura de um dado ficheiro indicado pelo utilizador (em formato *.ppm), identificar todas as fichas visíveis, classificá-las consoante o seu tipo (branca, vermelha, verde, azul, preta, “defeituosa” e “desconhecida”) e apresentar o respetivo somatório. Mais ainda, o seguinte conjunto de informação deverá ser disponibilizado após o processamento da imagem:
* Número total de fichas visíveis na imagem;
* Número de fichas por tipo (branca, vermelha, azul, ou preta, e sendo que qualquer ficha “alterada” é
considerada defeituosa e as “blinds” são consideradas “desconhecidas”);
* Área e perímetro (em pixéis) de todas as fichas identificadas;
* Desenho, sobre a imagem, da localização (área delimitadora) e centro de gravidade de cada ficha, bem como indicação do seu respetivo tipo;
* Somatório do valor das fichas válidas identificadas.