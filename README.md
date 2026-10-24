# LaClaw Lab

**Português (Brasil)** | [English](README.en.md)

Biblioteca de computação gráfica com OOP em **C11** e simulador 3D de uma máquina
de prêmios, usando **OpenGL e freeglut**. O braço tem base, ombro, cotovelo, punho
e dois dedos articulados. Controle as juntas, capture três alienígenas e solte-os na
bandeja. A captura é determinística; o modo escolhido define o tempo e a intensidade dos quiques.

O visual é inspirado na máquina do **Pizza Planet, de Toy Story**.

Projeto para o **cenário 3 — Simulador de Braço Robótico**, CG 2026/2,
CEFET/RJ, UnED Petrópolis. IK não foi implementada.

A arquitetura segue os padrões de `Geometry`, base incorporada, métodos com
`self` e fábricas de
[Enz0Moura/primeiro-trabalho-cg](https://github.com/Enz0Moura/primeiro-trabalho-cg/tree/c12a40e4254ea511ad8906883b9ca91b4a429dc0),
com matemática 3D, árvore de cena, destrutores e propriedade de memória explícita.

## Compilar e executar

Dependências: compilador C11, Make, headers/bibliotecas OpenGL e freeglut.
No CachyOS/Arch: `sudo pacman -S --needed base-devel freeglut libglvnd`.
É necessário um display gráfico para abrir as janelas; os testes não abrem janela.

```sh
make lib       # build/libcg.a, independente da aplicação
make demo      # compila build/library_demo
make example   # executa o exemplo independente
make run       # compila e executa build/prize_machine
make test      # matemática, estados, trajetórias e falhas de alocação
make sanitize  # os mesmos testes com Clang, ASan e UBSan
make smoke     # sessão gráfica automática completa; requer display
make clean     # remove build/
```

`make` compila os dois executáveis. O simulador e o exemplo vinculam o mesmo
`build/libcg.a`. Escolha um modo no menu com o mouse ou **1–4**.
Para jogar: clique num prêmio; escolha a junta com **1–4**;
use **←/→** ou **+/−**; **C** fecha, **O** abre, **H** mostra a ajuda.
Botão direito e roda controlam a câmera. **R** reinicia; **D** mostra a depuração.
Arraste com a roda pressionada para deslocar a visão. **F** alterna a câmera
livre: botão direito olha ao redor e a roda avança ou recua. **Home** restaura a câmera.

## Modos de jogo

| Modo | Tempo | Colisões |
| --- | --- | --- |
| Sandbox | Sem limite | Quiques suaves para praticar |
| Fácil | 4 minutos | Quiques leves e maior amortecimento |
| Médio | 3 minutos | Quiques moderados |
| Difícil | 2 minutos | Quiques fortes e menor amortecimento |

Aliens soltos quicam no piso, na cabine e entre si. A entrega conta apenas quando
o alien para diretamente na bandeja, inteiramente dentro dela. **M** abre o menu;
escolher um modo inicia uma nova partida. Menu e ajuda pausam a simulação e o
cronômetro. **R** reinicia o modo atual. Vitória ou tempo esgotado encerram a rodada.

## Documentação

Veja o [manual do usuário e primeira entrega passo a passo](docs/MANUAL_DO_USUARIO.md).

Validado em CachyOS Linux, freeglut 3.8.0 e OpenGL de compatibilidade 4.6/NVIDIA.
Usa AABBs conservadoras, gravidade, colisões com quique e atrito simplificado numa
região limitada da cabine. Não implementa autocolisão do braço nem rotação física dos prêmios.
