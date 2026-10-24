# Manual do usuário — LaClaw Lab

**Português (Brasil)** | [English](USER_MANUAL.md)

[Voltar ao README](../README.md)

## 1. Objetivo

Pegue os três prêmios — Coral, Azul e Amarelo — com o braço articulado e solte-os
na bandeja verde. Cada prêmio conta uma vez, depois de chegar à bandeja solto.
Ao chegar a **3/3**, a coleção está completa. Não há tempo limite, sorteio de
captura ou chance de o prêmio escapar durante o transporte.

O jogador controla os ângulos das juntas. Clicar num prêmio apenas o destaca;
o braço não se move automaticamente até ele. O tutorial da seção 5 oferece
uma sequência de ângulos já verificada.

## 2. Preparação e execução

O ambiente efetivamente verificado foi **CachyOS Linux (base Arch)**, em sessão
com display X11/XWayland `:0`, GCC 16.2.1, Clang 23.1.3, Make 4.4.1,
freeglut 3.8.0, libglvnd 1.7.0 e NVIDIA RTX 2060 com OpenGL 4.6 de
compatibilidade, driver 615.78.08. Não foi verificado suporte a Windows ou macOS.
O programa usa recursos do pipeline de compatibilidade, não exige OpenGL 4.6.

No CachyOS/Arch, os pacotes necessários podem ser instalados com:

```sh
sudo pacman -S --needed base-devel freeglut libglvnd
```

Essas dependências já estavam instaladas durante o desenvolvimento; o comando
acima indica como obtê-las. Abra um terminal **na pasta do projeto**, aquela que
contém `Makefile`, e execute:

```sh
make
./build/prize_machine
```

Ou use apenas `make run`. Não execute o programa com `sudo`.
A janela abre em 1280 × 800. Recomenda-se pelo menos 1100 × 720; abaixo disso,
textos e controles podem ficar próximos demais. A câmera se adapta à proporção
da área 3D, e o painel se compacta verticalmente.

Para conhecer apenas a biblioteca:

```sh
make lib
make example
```

O exemplo mostra um cubo alongado e um cilindro que acompanha a rotação de seu
pai. Nesse exemplo, as setas controlam a câmera, `+`/`-` controlam o zoom e
`Esc` encerra. Os comandos do simulador são os da próxima seção.

## 3. Todos os controles do simulador

| Ação | Comando |
| --- | --- |
| Selecionar um prêmio disponível | Clique esquerdo diretamente no alienígena |
| Selecionar base / ombro / cotovelo / punho | `1` / `2` / `3` / `4` |
| Selecionar junta pelo painel | Clique na linha da junta |
| Diminuir / aumentar o ângulo continuamente | Segure `←` / `→` |
| Diminuir / aumentar o alvo em 5° | Toque `-` / `+` (ou `=` para aumentar) |
| Ajustar o alvo em −1° / +1° | `[` / `]` |
| Ajustar 5° pelo mouse | Botões `−` / `+` na linha da junta |
| Fechar a garra | `C` ou botão **C FECHAR** |
| Abrir e soltar o prêmio | `O` ou botão **O ABRIR** |
| Alternar fechar / abrir | `Espaço` |
| Girar a câmera | Arraste com o botão direito na cena |
| Aproximar / afastar a câmera | Roda do mouse na cena |
| Restaurar a câmera | `Home` |
| Ajuda | `H`, `F1` ou botão **H AJUDA** |
| Visualização de depuração | `D` |
| Reiniciar a cena e a câmera | `R` ou botão **R REINICIAR** |
| Sair | `Esc` ou fechar a janela |

As letras funcionam em maiúsculas e minúsculas. Para os incrementos de 5° e 1°,
pressione e solte a tecla a cada passo; segurar `+` não repete os passos.
Use as setas para movimento contínuo. Soltá-las interrompe esse movimento.
Os botões da câmera e da seleção são diferentes para não misturar as ações.
A ajuda aberta impede selecionar objetos atrás do painel de ajuda.

## 4. O que aparece na tela

**Cena:** a máquina tem o tema Pizza Planet, inspirado na referência de Toy Story:
foguete vermelho, topo amarelo, aletas laterais, faixa azul e estrelas. O braço
metálico tem articulações vermelhas. Os três prêmios são alienígenas verdes de
três olhos, antena e uniforme azul. **Coral, Azul e Amarelo** identificam suas
insígnias e os rótulos na tela; a pele dos três é verde. A plataforma verde
com três espaços continua sendo a bandeja de entrega. A abertura escura na
parte externa do foguete é decorativa. A frente aberta permite ver o braço.

**Seleção:** um contorno verde-limão acompanha o alienígena selecionado e sua linha fica
destacada na coleção. Clicar num espaço vazio ou numa superfície que bloqueia
um prêmio limpa a seleção. O destaque não dá alcance extra à garra.

**Tripulação:** mostra a contagem e os estados `Disponivel`, `Na garra`, `Em queda`
ou `Entregue`. Um prêmio entregue permanece visível na bandeja, mas não pode ser
capturado novamente. Seus rótulos na cena desaparecem para não encobrir os demais.

**Juntas:** a linha verde-limão é a junta ativa. O número grande é o ângulo
**atual**, em graus; o intervalo entre colchetes é o limite. Os ângulos mudam
gradualmente depois de pressionar os botões. Aguarde o valor desejado antes de
executar a próxima etapa do tutorial.

| Junta | Inicial | Limites |
| --- | ---: | ---: |
| Base | 0° | −110° a 65° |
| Ombro | −25° | −85° a 25° |
| Cotovelo | −85° | −135° a −15° |
| Punho | −70° | −110° a 80° |
| Dedos | 40°, abertos | 0° a 40° |

Uma colisão pode impedir o avanço antes do limite angular. O intervalo indica a
capacidade da junta, não uma garantia de que toda combinação das juntas cabe na cabine.

**Garra:** a barra indica abertura. Durante a captura, os dedos param em torno
de 20°, sem esmagar o alienígena. `Captura possivel` indica alinhamento suficiente;
`Garra ocupada` confirma que o prêmio acompanha o braço.

**Rodapé:** informa captura, soltura, bloqueio e entrega. `Movimento bloqueado`
significa que a última pequena tentativa de avanço foi rejeitada. Mude a junta
no sentido contrário ou ajuste outra junta para sair da posição.

**Depuração (D):** mostra caixas, eixos coloridos, posição da garra e o número
de movimentos rejeitados. A caixa rosa marca a tolerância para o **centro** do
prêmio. O volume verde marca a entrega. A caixa externa marca os limites da
cabine. Esse modo ajuda a estudar e não é necessário para jogar.

## 5. Primeira entrega, passo a passo

Esta sequência foi executada pelos testes e pela sessão gráfica. Faça-a a partir
de `R`, com a ajuda fechada. **Aguarde cada ângulo terminar de mudar**. Não altere
o cotovelo: ele permanece em −85°. Os sinais negativos fazem parte dos valores.

1. Clique no alienígena **Coral**, identificado pelo rótulo coral próximo da frente. Confira seu contorno.
2. Pressione `1` e toque `-` **8 vezes**. A base vai de 0° a **−40°**.
3. Pressione `4` e toque `+` (ou `=`) **4 vezes**. O punho vai a **−50°**.
4. Pressione `2` e toque `-` **4 vezes**. O ombro vai a **−45°**.
   A garra aberta fica ao redor do prêmio; aparece `Captura possivel`.
5. Pressione `C`. Aguarde `Premio capturado!` e o estado `Na garra`.
6. Pressione `2` e toque `+` **4 vezes**. O ombro sobe até **−25°**, erguendo o prêmio.
7. Pressione `4` e toque `-` **4 vezes**. O punho volta a **−70°**.
8. Pressione `1` e toque `-` **7 vezes**. A base vai de −40° a **−75°**,
   posicionando a carga sobre o espaço direito da bandeja.
9. Pressione `O`. O prêmio cai verticalmente e para na bandeja.
   A contagem passa a **1/3** após o contato. Espere a garra abrir até 40°.

O braço não usa a seleção para alcançar o objeto. Por isso, selecionar outro
alienígena não substitui os ajustes do tutorial. Se houver um erro, pressione `R`
e repita a sequência; câmera e ângulos retornam ao início.

### Entregar os outros dois sem reiniciar

Repita a mesma ordem: base para coleta → punho −50° → ombro −45° → fechar →
ombro −25° → punho −70° → base para entrega → abrir e esperar.

| Prêmio | Base na coleta | Base na entrega | Cotovelo |
| --- | ---: | ---: | ---: |
| Coral | −40° | −75° | −85° |
| Azul | 0° | −90° | −85° |
| Amarelo | 40° | −105° | −85° |

Depois do Coral, selecione o Azul e aumente a base de −75° até 0°: **15 toques**
em `+`. Depois de levantar o Azul, vá de 0° até −90°: **18 toques** em `-`.

Depois do Azul, selecione o Amarelo e aumente a base de −90° até 40°:
**26 toques** em `+`. Depois de levantar o Amarelo, vá de 40° até −105°:
**29 toques** em `-`.

As diferentes posições de entrega deixam espaço para os três alienígenas.
Soltar todos exatamente no mesmo lugar pode empilhá-los; um alienígena apoiado em
outro não conta como entregue. Use os três espaços e a tabela acima.

### Experimentar uma colisão

Pressione `R`, depois `2`, e segure `←`. A tentativa de continuar abaixando o
ombro acaba bloqueada; a junta para antes de −85°. Ative `D` para ver as caixas
e o contador de movimentos rejeitados. Pressione `R` para começar o tutorial
novamente. A câmera pode atravessar a cabine; ela é apenas um ponto de observação.

## 6. Regras do jogo

- A captura exige garra vazia, prêmio disponível próximo do centro da garra,
  tamanho compatível e caminho desobstruído. Fechar longe não captura nada.
- Os dedos precisam estar abertos para receber o alienígena. A seleção é opcional
  para a captura: vale a proximidade real, não o nome destacado.
- Depois de capturado, o prêmio acompanha posição e orientação da garra sem
  um salto de alinhamento. O fechamento para quando o espaço é suficiente.
- O braço e a carga não podem atravessar piso, fundo, colunas, batente,
  bandeja, demais prêmios ou os limites internos da cabine. Os lados abertos
  permitem olhar, mas continuam sendo limites da área de jogo.
- Algumas caixas são maiores que a forma visível quando as peças estão
  inclinadas. Pode haver bloqueio um pouco antes do contato aparente.
- Abrir solta o prêmio na posição atual; ele cai verticalmente e mantém a
  orientação, sem quique ou rolamento. Fora da bandeja, fica disponível no
  piso ou em outro apoio. Ele não ganha pontos por isso.
- Para contar, o prêmio deve pousar diretamente sobre a bandeja e caber
  inteiramente nela. Passar por cima ainda preso não conta. Pousar parcialmente
  na borda ou em outro prêmio também não conta.
- A entrega é irreversível até reiniciar. Cada prêmio incrementa uma vez.
- `R` restaura prêmios, ângulos, seleção, contagem, câmera e junta ativa.
  As opções de mostrar ajuda e depuração conservam sua preferência atual.

## 7. Problemas comuns

**“GL/freeglut.h: No such file” ou “cannot find -lglut”:** faltam os headers ou
a biblioteca freeglut. Instale os pacotes da seção 2 e compile novamente.

**“failed to open display”:** execute em uma sessão gráfica local com acesso
ao display. Um terminal remoto sem encaminhamento gráfico não abre a janela.
`make test` e `make sanitize` continuam funcionando sem janela.

**A janela aparece, mas o braço não responde:** clique para dar foco à janela,
selecione uma junta com `1`–`4` e use as setas. Para incrementos, solte e
pressione de novo. Confira se o rodapé informa um bloqueio ou limite.

**Não consigo selecionar o prêmio:** clique no alienígena, não no texto suspenso.
Feche a ajuda. Uma coluna, peça do braço ou outro prêmio pode estar na frente;
gire a câmera com o botão direito. `Home` restaura a vista inicial.

**A garra fecha sem pegar:** abra com `O` e aguarde 40°. Ajuste as juntas até
`Captura possivel`. Se já estiver muito fechada, ela não consegue receber o alienígena.

**O prêmio ficou difícil de pegar após cair:** a orientação é preservada e
algumas posições inclinadas não cabem na garra. Reposicione-a ou use `R`.
Reiniciar sempre retorna aos três prêmios em posições utilizáveis.

**Prêmio na bandeja, mas sem ponto:** ele deve estar solto, apoiado diretamente
na bandeja e totalmente dentro dela. Uma pilha ou a borda não é uma entrega.

**Textos pequenos ou sobrepostos:** amplie a janela para 1280 × 800.
Os textos da interface usam caracteres sem acentos por compatibilidade com a
fonte bitmap do freeglut; isso não indica problema na instalação.

## 8. Limitações e verificações

Esta é uma simulação didática de cinemática direta. Não inclui IK, física
realista dos dedos, autocolisão entre segmentos, deformação de pelúcia, som,
persistência entre execuções ou salvamento de partida. A câmera pode mostrar
o verso das peças; não há vidro transparente nem sombras projetadas.

As três rotas da tabela foram verificadas com as colisões ativadas. Os testes
de lógica e memória podem ser executados com `make test` e `make sanitize`.

Para reproduzir a sessão automática de verificação:

```sh
make smoke
```

Ela abre e fecha o exemplo e depois a máquina, exercita os controles e deixa
capturas reais em `build/smoke_*.ppm`. É uma ferramenta de teste; o modo normal
de jogo, iniciado com `make run`, permanece inteiramente manual.
