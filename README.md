# LaClaw Lab

**Português (Brasil)** | [English](README.en.md)

Biblioteca de computação gráfica com OOP em **C11** e simulador 3D de uma máquina
de prêmios, usando **OpenGL e freeglut**. O braço tem base, ombro, cotovelo, punho
e dois dedos articulados. Controle as juntas, capture três alienígenas e solte-os na
bandeja. A captura é determinística; não há limite de tempo.

O visual é inspirado na máquina do **Pizza Planet, de Toy Story**, conforme a
referência enviada: foguete vermelho/amarelo, aletas, estrelas e alienígenas
verdes de três olhos. As formas são geradas em C, sem modelos ou texturas externos.

Projeto para o **cenário 3 — Simulador de Braço Robótico**, CG 2026/2,
CEFET/RJ, UnED Petrópolis. Os requisitos foram extraídos do briefing recebido;
o PDF original não foi fornecido. IK é opcional e não foi implementada.

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
`build/libcg.a`. Para jogar: clique num prêmio; escolha a junta com **1–4**;
use **←/→** ou **+/−**; **C** fecha, **O** abre, **H** mostra a ajuda.
Botão direito e roda controlam a câmera. **R** reinicia; **D** mostra a depuração.

## Documentação

Veja o [manual do usuário e primeira entrega passo a passo](docs/MANUAL_DO_USUARIO.md).

Validado em CachyOS Linux, freeglut 3.8.0 e OpenGL de compatibilidade 4.6/NVIDIA.
Usa AABBs conservadoras, queda vertical simplificada e uma região limitada da
cabine. Não implementa autocolisão do braço nem física de corpos rígidos.
